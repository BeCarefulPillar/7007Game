// Copyright (C) 2016 Joywinds Inc.

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SkipList.h"

namespace joywinds {

    typedef struct sdsObj {
        char* ptr;
        size_t len;
    } sdsObj;

    typedef sdsObj* sds;

    sds sdsnew(const char* ptr, size_t len) {
        assert(ptr != NULL);
        assert(len > 0);
        sds s = (sds)malloc(sizeof(*s));
        s->ptr = (char*)malloc(len);
        memcpy(s->ptr, ptr, len);
        s->len = len;
        return s;
    }

    int sdscmp(sds s1, sds s2) {
        int cmp = memcmp(s1->ptr, s2->ptr, s1->len < s2->len ? s1->len : s2->len);
        if (cmp == 0) return s1->len - s2->len;
        return cmp;
    }

    void sdsfree(sds s) {
        free(s->ptr);
        free(s);
    }

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P 0.25

    typedef struct zskiplistLevel {
        struct zskiplistNode *forward;
        unsigned int span;
    } zskiplistLevel;

    typedef struct zskiplistNode {
        sds ele;
        double score;
        struct zskiplistNode *backward;
        struct zskiplistLevel level[];
    } zskiplistNode;

    typedef struct zskiplist {
        struct zskiplistNode *header, *tail;
        unsigned long length;
        int level;
    } zskiplist;

    /* Struct to hold a inclusive/exclusive range spec by score comparison. */
    typedef struct {
        double min, max;
        int minex, maxex; /* are min or max exclusive? */
    } zrangespec;

    /* Create a skiplist node with the specified number of levels.
     * The SDS string 'ele' is referenced by the node after the call. */
    zskiplistNode *zslCreateNode(int level, double score, sds ele) {
        zskiplistNode *zn = (zskiplistNode*)malloc(sizeof(*zn) + level * sizeof(struct zskiplistLevel));
        zn->score = score;
        zn->ele = ele;
        return zn;
    }

    /* Free the specified skiplist node. The referenced SDS string representation
     * of the element is freed too, unless node->ele is set to NULL before calling
     * this function. */
    void zslFreeNode(zskiplistNode *node) {
        sdsfree(node->ele);
        free(node);
    }

    /* Returns a random level for the new skiplist node we are going to create.
     * The return value of this function is between 1 and ZSKIPLIST_MAXLEVEL
     * (both inclusive), with a powerlaw-alike distribution where higher
     * levels are less likely to be returned. */
    int zslRandomLevel(void) {
        int level = 1;
        while ((rand() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
            level += 1;
        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    /* Create a new skiplist. */
    zskiplist *zslCreate(void) {
        int j;
        zskiplist *zsl;

        zsl = (zskiplist*)malloc(sizeof(*zsl));
        zsl->level = 1;
        zsl->length = 0;
        zsl->header = zslCreateNode(ZSKIPLIST_MAXLEVEL, 0, NULL);
        for (j = 0; j < ZSKIPLIST_MAXLEVEL; j++) {
            zsl->header->level[j].forward = NULL;
            zsl->header->level[j].span = 0;
        }
        zsl->header->backward = NULL;
        zsl->tail = NULL;
        return zsl;
    }

    /* Free a whole skiplist. */
    void zslFree(zskiplist *zsl) {
        zskiplistNode *node = zsl->header->level[0].forward, *next;

        free(zsl->header);
        while (node) {
            next = node->level[0].forward;
            zslFreeNode(node);
            node = next;
        }
        free(zsl);
    }

    /* Insert a new node in the skiplist. Assumes the element does not already
     * exist (up to the caller to enforce that). The skiplist takes ownership
     * of the passed SDS string 'ele'. */
    zskiplistNode *zslInsert(zskiplist *zsl, double score, sds ele) {
        zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
        unsigned int rank[ZSKIPLIST_MAXLEVEL];
        int i, level;

        //serverAssert(!isnan(score));
        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            /* store rank that is crossed to reach the insert position */
            rank[i] = i == (zsl->level - 1) ? 0 : rank[i + 1];
            while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                (x->level[i].forward->score == score &&
                    sdscmp(x->level[i].forward->ele, ele) < 0))) {
                rank[i] += x->level[i].span;
                x = x->level[i].forward;
            }
            update[i] = x;
        }
        /* we assume the element is not already inside, since we allow duplicated
         * scores, reinserting the same element should never happen since the
         * caller of zslInsert() should test in the hash table if the element is
         * already inside or not. */
        level = zslRandomLevel();
        if (level > zsl->level) {
            for (i = zsl->level; i < level; i++) {
                rank[i] = 0;
                update[i] = zsl->header;
                update[i]->level[i].span = zsl->length;
            }
            zsl->level = level;
        }
        x = zslCreateNode(level, score, ele);
        for (i = 0; i < level; i++) {
            x->level[i].forward = update[i]->level[i].forward;
            update[i]->level[i].forward = x;

            /* update span covered by update[i] as x is inserted here */
            x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
            update[i]->level[i].span = (rank[0] - rank[i]) + 1;
        }

        /* increment span for untouched levels */
        for (i = level; i < zsl->level; i++) {
            update[i]->level[i].span++;
        }

        x->backward = (update[0] == zsl->header) ? NULL : update[0];
        if (x->level[0].forward)
            x->level[0].forward->backward = x;
        else
            zsl->tail = x;
        zsl->length++;
        return x;
    }

    /* Internal function used by zslDelete, zslDeleteByScore and zslDeleteByRank */
    void zslDeleteNode(zskiplist *zsl, zskiplistNode *x, zskiplistNode **update) {
        int i;
        for (i = 0; i < zsl->level; i++) {
            if (update[i]->level[i].forward == x) {
                update[i]->level[i].span += x->level[i].span - 1;
                update[i]->level[i].forward = x->level[i].forward;
            } else {
                update[i]->level[i].span -= 1;
            }
        }
        if (x->level[0].forward) {
            x->level[0].forward->backward = x->backward;
        } else {
            zsl->tail = x->backward;
        }
        while (zsl->level > 1 && zsl->header->level[zsl->level - 1].forward == NULL)
            zsl->level--;
        zsl->length--;
    }

    /* Delete an element with matching score/element from the skiplist.
     * The function returns 1 if the node was found and deleted, otherwise
     * 0 is returned.
     *
     * If 'node' is NULL the deleted node is freed by zslFreeNode(), otherwise
     * it is not freed (but just unlinked) and *node is set to the node pointer,
     * so that it is possible for the caller to reuse the node (including the
     * referenced SDS string at node->ele). */
    int zslDelete(zskiplist *zsl, double score, sds ele, zskiplistNode **node) {
        zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
        int i;

        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                (x->level[i].forward->score == score &&
                    sdscmp(x->level[i].forward->ele, ele) < 0))) {
                x = x->level[i].forward;
            }
            update[i] = x;
        }
        /* We may have multiple elements with the same score, what we need
         * is to find the element with both the right score and object. */
        x = x->level[0].forward;
        if (x && score == x->score && sdscmp(x->ele, ele) == 0) {
            zslDeleteNode(zsl, x, update);
            if (!node)
                zslFreeNode(x);
            else
                *node = x;
            return 1;
        }
        return 0; /* not found */
    }

    /* Find the rank for an element by both score and key.
     * Returns 0 when the element cannot be found, rank otherwise.
     * Note that the rank is 1-based due to the span of zsl->header to the
     * first element. */
    unsigned long zslGetRank(zskiplist *zsl, double score, sds ele) {
        zskiplistNode *x;
        unsigned long rank = 0;
        int i;

        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                (x->level[i].forward->score == score &&
                    sdscmp(x->level[i].forward->ele, ele) <= 0))) {
                rank += x->level[i].span;
                x = x->level[i].forward;
            }

            /* x might be equal to zsl->header, so test if obj is non-NULL */
            if (x->ele && sdscmp(x->ele, ele) == 0) {
                return rank;
            }
        }
        return 0;
    }

    /* Finds an element by its rank. The rank argument needs to be 1-based. */
    zskiplistNode* zslGetElementByRank(zskiplist *zsl, unsigned long rank) {
        zskiplistNode *x;
        unsigned long traversed = 0;
        int i;

        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            while (x->level[i].forward && (traversed + x->level[i].span) <= rank) {
                traversed += x->level[i].span;
                x = x->level[i].forward;
            }
            if (traversed == rank) {
                return x;
            }
        }
        return NULL;
    }

    int zslValueGteMin(double value, zrangespec *spec) {
        return spec->minex ? (value > spec->min) : (value >= spec->min);
    }

    int zslValueLteMax(double value, zrangespec *spec) {
        return spec->maxex ? (value < spec->max) : (value <= spec->max);
    }

    /* Returns if there is a part of the zset is in range. */
    int zslIsInRange(zskiplist *zsl, zrangespec *range) {
        zskiplistNode *x;

        /* Test for ranges that will always be empty. */
        if (range->min > range->max ||
            (range->min == range->max && (range->minex || range->maxex)))
            return 0;
        x = zsl->tail;
        if (x == NULL || !zslValueGteMin(x->score, range))
            return 0;
        x = zsl->header->level[0].forward;
        if (x == NULL || !zslValueLteMax(x->score, range))
            return 0;
        return 1;
    }

    /* Find the first node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range) {
        zskiplistNode *x;
        int i;

        /* If everything is out of range, return early. */
        if (!zslIsInRange(zsl, range)) return NULL;

        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            /* Go forward while *OUT* of range. */
            while (x->level[i].forward &&
                !zslValueGteMin(x->level[i].forward->score, range))
                x = x->level[i].forward;
        }

        /* This is an inner range, so the next node cannot be NULL. */
        x = x->level[0].forward;
        assert(x != NULL);

        /* Check if score <= max. */
        if (!zslValueLteMax(x->score, range)) return NULL;
        return x;
    }

    /* Find the last node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range) {
        zskiplistNode *x;
        int i;

        /* If everything is out of range, return early. */
        if (!zslIsInRange(zsl, range)) return NULL;

        x = zsl->header;
        for (i = zsl->level - 1; i >= 0; i--) {
            /* Go forward while *IN* range. */
            while (x->level[i].forward &&
                zslValueLteMax(x->level[i].forward->score, range))
                x = x->level[i].forward;
        }

        /* This is an inner range, so this node cannot be NULL. */
        assert(x != NULL);

        /* Check if score >= min. */
        if (!zslValueGteMin(x->score, range)) return NULL;
        return x;
    }

    SkipList::SkipList() {
        sl_ = zslCreate();
    }

    SkipList::~SkipList() {
        zslFree(sl_);
        sl_ = 0;
    }

    void SkipList::insert(const std::string& key, const std::string& value, int score) {
        assert(!key.empty());
        sds s = sdsnew(key.data(), key.size());
        lock_.lock();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            zslDelete(sl_, (double)it->second.score, s, NULL);
            it->second.score = score;
            it->second.value = value;
        } else {
            Element element;
            element.score = score;
            element.key = key;
            element.value = value;
            dict_[key] = element;
        }
        zslInsert(sl_, (double)score, s);
        lock_.unlock();
    }

    void SkipList::remove(const std::string& key) {
        assert(!key.empty());
        sds s = sdsnew(key.data(), key.size());
        lock_.lock();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            zslDelete(sl_, (double)it->second.score, s, NULL);
            dict_.erase(it);
        }
        lock_.unlock();
    }

    bool SkipList::addScore(const std::string& key, int score) {
        bool exists = false;
        sds s = sdsnew(key.data(), key.size());
        lock_.lock();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            exists = true;
            zslDelete(sl_, (double)it->second.score, s, NULL);
            it->second.score += score;
            if (it->second.score < 0) {
                it->second.score = 0;
            }
            zslInsert(sl_, (double)it->second.score, s);
        }
        lock_.unlock();
        return exists;
    }

    bool SkipList::addScoreWithValue(const std::string& key, const std::string& value, int score) {
        bool exists = false;
        sds s = sdsnew(key.data(), key.size());
        lock_.lock();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            exists = true;
            zslDelete(sl_, (double)it->second.score, s, NULL);
            it->second.score += score;
            if (it->second.score < 0) {
                it->second.score = 0;
            }
            it->second.value = value;
            zslInsert(sl_, (double)it->second.score, s);
        }
        lock_.unlock();
        return exists;
    }

    bool SkipList::getRank(const std::string& key, bool reverse, int& rank) {
        bool exist = false;
        sdsObj so{ const_cast<char*>(key.data()), key.size() };
        lock_.lockRead();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            exist = true;
            rank = (int)zslGetRank(sl_, (double)it->second.score, &so);
            if (reverse) rank = sl_->length - rank + 1;
        }
        lock_.unlockRead();
        return exist;
    }

    bool SkipList::getScore(const std::string& key, int& score) {
        bool exist = false;
        lock_.lockRead();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            exist = true;
            score = it->second.score;
        }
        lock_.unlockRead();
        return exist;
    }

    bool SkipList::getElementByKey(const std::string& key, Element& result) {
        bool exist = false;
        lock_.lockRead();
        auto it = dict_.find(key);
        if (it != dict_.end()) {
            exist = true;
            result.score = it->second.score;
            result.key = it->second.key;
            result.value = it->second.value;
        }
        lock_.unlockRead();
        return exist;
    }

    bool SkipList::getElementByRank(int rank, Element& result, bool reverse) {
        bool exist = false;
        lock_.lockRead();
        if (reverse) rank = sl_->length - rank + 1;
        zskiplistNode* node = zslGetElementByRank(sl_, (unsigned long)rank);
        if (node != NULL) {
            exist = true;
            result.score = node->score;
            result.key.assign(node->ele->ptr, node->ele->len);
            auto it = dict_.find(result.key);
            assert(it != dict_.end());
            result.value = it->second.value;
        }
        lock_.unlockRead();
        return exist;
    }

    void SkipList::getElementsByRank(const std::vector<int>& ranks, std::vector<Element>& results, bool reverse) {
        lock_.lockRead();
        for (size_t i = 0; i < ranks.size(); i++) {
            int rank = reverse ? sl_->length - ranks[i] + 1 : ranks[i];
            zskiplistNode* node = zslGetElementByRank(sl_, (unsigned long)rank);
            if (node == NULL) break;
            Element result;
            result.score = node->score;
            result.key.assign(node->ele->ptr, node->ele->len);
            auto it = dict_.find(result.key);
            assert(it != dict_.end());
            result.value = it->second.value;
            results.push_back(result);
        }
        lock_.unlockRead();
    }

    void SkipList::getElementsByRange(int start, int end, std::vector<Element>& results, bool reverse) {
        lock_.lockRead();
        zskiplist* zsl = sl_;
        /* Sanitize indexes. */
        int llen = zsl->length;
        if (start < 0) start = llen + start;
        if (end < 0) end = llen + end;
        if (start < 0) start = 0;
        /* Invariant: start >= 0, so this test will be true when end < 0.
         * The range is empty when start > end or start >= length. */
        if (start > end || start >= llen) {
            lock_.unlockRead();
            return;
        }
        if (end >= llen) end = llen - 1;
        int rangelen = (end - start) + 1;
        results.reserve(rangelen);

        zskiplistNode *ln;
        sds ele;
        /* Check if starting point is trivial, before doing log(N) lookup. */
        if (reverse) {
            ln = zsl->tail;
            if (start > 0) {
                ln = zslGetElementByRank(zsl, llen - start);
            }
        } else {
            ln = zsl->header->level[0].forward;
            if (start > 0) {
                ln = zslGetElementByRank(zsl, start + 1);
            }
        }

        while (rangelen--) {
            assert(ln != NULL);
            ele = ln->ele;
            Element e;
            e.score = ln->score;
            e.key.assign(ele->ptr, ele->len);
            auto it = dict_.find(e.key);
            assert(it != dict_.end());
            e.value = it->second.value;
            results.push_back(e);
            ln = reverse ? ln->backward : ln->level[0].forward;
        }
        lock_.unlockRead();
    }

    int SkipList::getTotal() {
        lock_.lockRead();
        int total = sl_->length;
        lock_.unlockRead();
        return total;
    }

} // namespace joywinds
using UnityEngine;
using System.Collections;

public class testCode : MonoBehaviour {

    private GameObject mItem;
    private GameObject[] Items = new GameObject[11];
    private Vector3 Pos = new Vector3(0, 0, 0);
    private bool begin = false;

    void Start() {
        mItem = transform.Find("Item").gameObject;
        for (int i = 9; i >= 0; i--) {
            Clone(i);
        }
        begin = true;
    }

    void Clone(int i) {
        GameObject _clone = Instantiate(mItem) as GameObject;
        Pos.x = 100 * i;
        _clone.transform.SetParent(gameObject.transform);
        _clone.transform.localPosition = Pos;
        _clone.transform.localScale = Vector3.one;
        _clone.SetActive(true);
        Items[i] = _clone;
    }

    void Update() {
        if (begin) {
            float max = 0;
            int j = 0;
            for (int i = 0; i < 10; i++) {
                Vector3 Ipos = Items[i].transform.localPosition;
                float factor = Mathf.Abs(Ipos.x + gameObject.transform.parent.transform.localPosition.x);
                factor = 1 - factor / 1000;
                Vector3 p = Vector3.one * factor;
                p.x = 1;
                Items[i].transform.localScale = p;
                if (factor > max) {
                    max = factor;
                    j = i;
                }
            }
            for (int i = 0; i < 10; i++) {
                if (j != i) {
                    Items[i].GetComponent<UISprite>().depth = 1;
                } else {
                    Items[i].GetComponent<UISprite>().depth = 2;
                }
            }
        }
    }

}

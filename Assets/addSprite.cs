using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class addSprite : MonoBehaviour
{
    private Texture2D t2d;
    // Start is called before the first frame update
    void Start()
    {
        float x;
        x = -9.6f;
        createSprit(x, -5.4f);
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void createSprit(float x, float y) {
        
        GameObject gameObject = new GameObject();
        SpriteRenderer spr = gameObject.AddComponent<SpriteRenderer>();

        Rect rect = new Rect();
        rect.x = 0f;
        rect.y = 0f;
        rect.width = 1920;
        rect.height = 1080;
        Sprite sp = Sprite.Create(t2d, rect, new Vector2(0f, 0f));

        spr.sprite = sp;
        gameObject.transform.position = new Vector3(x, y, 1f);
    }

}

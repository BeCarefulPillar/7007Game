
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class draw : MonoBehaviour {
    // Start is called before the first frame update
    void Start() {
        //DrawRect(new Rect(0, 0, 1000, 1000), Color.red);
        
    }

    // Update is called once per frame
    void Update() {


        DrawRect(new Rect(0,0,100,100),Color.red);
    }

    public static void DrawRect(Rect rect, Color color) {

        Vector3[] line = new Vector3[5];

        line[0] = new Vector3(rect.x, rect.y, 0);

        line[1] = new Vector3(rect.x + rect.width, rect.y, 0);

        line[2] = new Vector3(rect.x + rect.width, rect.y + rect.height, 0);

        line[3] = new Vector3(rect.x, rect.y + rect.height, 0);

        line[4] = new Vector3(rect.x, rect.y, 0);

        if (line != null && line.Length > 0) {
            DrawLineHelper(line, color);
        }

    }

    public static void DrawLineHelper(Vector3[] line, Color color) {
        for (var i = 0; i < line.Length - 1; i++) {
            Debug.DrawLine(line[i], line[i + 1], color);
        }
    }
}
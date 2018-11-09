using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class withCamera : MonoBehaviour {

    // Use this for initialization
    void Start() {

    }

    // Update is called once per frame
    void Update() {
        float h = Input.GetAxis("Horizontal");
        this.transform.Translate(new Vector3(h * 5 * Time.deltaTime, 0, 0));
    }

    private void OnBecameVisible() {
        print("摄像机视野内");
    }

    private void OnBecameInvisible() {
        print("在摄像机视野外");
    }
}

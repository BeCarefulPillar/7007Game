﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class playerMove : MonoBehaviour {
    private Animator anim;
    private string lastAnim;
    private string nextAnim;

    // Use this for initialization
    void Start() {
        anim = GetComponent<Animator>();
    }

    // Update is called once per frame
    void Update() {

        if (Input.GetKey(KeyCode.RightArrow ) || Input.GetKey(KeyCode.LeftArrow  )) {
            float xAxis = Input.GetAxis("Horizontal");
            Vector3 eulerAngles = transform.localEulerAngles;

            if (xAxis < 0f) {
                eulerAngles.y = 180f;
            } else if (xAxis > 0f) {
                eulerAngles.y = 0f;
            }

            transform.localRotation = Quaternion.Euler(eulerAngles);

            anim.SetFloat("walk", 1, 0.2f, Time.deltaTime);
        } else{
            anim.SetFloat("walk", 0, 0.2f, Time.deltaTime);
        }

        if (Input.GetKey(KeyCode.A)) {
            anim.SetFloat("attack", 1, 0.2f, Time.deltaTime);
        }else{
            anim.SetFloat("attack", 0, 0.2f, Time.deltaTime);
        }
    }

    void ChangeAnim(string nextAnim) {
        anim.SetFloat(lastAnim, 0);
        lastAnim = nextAnim;
        anim.SetFloat(nextAnim,1);
    }
}

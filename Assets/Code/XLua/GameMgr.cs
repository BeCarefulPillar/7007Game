using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;

public class GameMgr : MonoBehaviour {

    private void Awake() {
        gameObject.AddComponent<LuaMgr>();
    }

    // Use this for initialization
    void Start () {
        LuaMgr.Init();
        LuaMgr.Instance.LoadModule("main");
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}

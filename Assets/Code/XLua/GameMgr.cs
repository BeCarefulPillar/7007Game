using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;

public class GameMgr : MonoBehaviour {
    
    // Use this for initialization
    void Start () {
        LuaMgr.Init();
        LuaMgr.Instance.LoadModule("main");
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}

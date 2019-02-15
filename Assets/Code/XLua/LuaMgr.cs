using System.Collections;
using System.Collections.Generic;
using System.IO;
using System;
using UnityEngine;
using XLua;

public class LuaMgr : MonoBehaviour {
    public static LuaMgr Instance;
    public static LuaEnv luaEnv;
    public static readonly string editorLuaPath = "Assets/Code/Lua";

    private void Awake() {
        DontDestroyOnLoad(this);
        Instance = this;
        luaEnv = new LuaEnv();

        //设置xlua脚本路径 路径根据实际需求修改
        luaEnv.AddLoader(LuaMgr.MyLoader);
        //luaEnv.DoString(string.Format("package.path = '{0}/Code/Lua/?.lua'", Application.dataPath));
    }

    public void DoString(string str) {
        luaEnv.DoString(str);
    }

    private static byte[] MyLoader(ref string filePath) {
        string path = filePath.Replace(".", "/");

        var code = UnityEditor.AssetDatabase.LoadAssetAtPath(editorLuaPath + "/" + path + ".lua.txt", typeof(TextAsset)) as TextAsset;
        if (code != null) {
            return code.bytes;
        }
        return null;
    }
}

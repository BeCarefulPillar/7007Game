using UnityEngine;
using XLua;

public class LuaHelperManager {
    protected static LuaHelperManager mInstance;
    protected LuaHelperManager() {}
    private static GameObject mPopupParent = GameObject.Find("Canvas/Group_Popup");
    private static GameObject mHudParent = GameObject.Find("Canvas/Group_Hud");
    public static LuaHelperManager Instance {
        get {
            if(mInstance == null) {
                mInstance = new LuaHelperManager();
            }
            return mInstance;
        }
    }

    /// <summary>
    /// Xlua层加载UI面板
    /// </summary>
    /// <param name="path"> 路径 </param>
    /// <param name="OnCreate"> 创建出来的委托回调 </param>
    public void LoadPopup(string path, LuaTable model, LuaTable data = null, XLuaCustomExport.OnCreate onCreate = null) {
        Debug.Log("加载UI窗口 =========== " + path);
        GameObject obj =  GameObject.Instantiate(Resources.Load<GameObject>(path), mPopupParent.transform);
        if (obj != null) {
            obj.AddComponent<LuaBehaviour>();
        }else {
            return;
        }
        if (onCreate != null){
            onCreate(obj);
        }
        var view = obj.GetComponent<LuaBehaviour>();
        view.SetLuaModule(model, data);
    }

    public void LoadHud(string path, LuaTable model, XLuaCustomExport.OnCreate onCreate = null) {
        Debug.Log("加载UI窗口 =========== " + path);
        GameObject obj = GameObject.Instantiate(Resources.Load<GameObject>(path), mHudParent.transform);
        if (obj != null) {
            obj.AddComponent<LuaBehaviour>();
        } else {
            return;
        }
        if (onCreate != null) {
            onCreate(obj);
        }
        var view = obj.GetComponent<LuaBehaviour>();
        view.SetLuaModule(model);
    }
}

/// <summary>
/// XLua的自定义拓展
/// </summary>
/// 
public static class XLuaCustomExport {
    [CSharpCallLua]
    public delegate void OnCreate(GameObject obj);
}


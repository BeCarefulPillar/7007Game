using System.Collections;
using UnityEngine;

public class LuaMgr : MonoBehaviour {

    public static readonly string targetLuacPath = "Luac"; // relative to assets/resources/
    public static readonly string editorLuaPath = "Assets/Code/Lua";
    public static readonly string password = "12345678";
    public static readonly float GC_INTERVAL = 60f;

    public static LuaMgr Instance {
        get {
            return s_instance;
        }
    }

    public XLua.LuaEnv Env {
        get; private set;
    }

    static LuaMgr s_instance;
    float m_lastGCTime = 0;

    void OnDestroy() {
        DeInit();
        s_instance = null;
    }

    public static void Init() {
        if (s_instance == null) {
            var go = new GameObject("__LuaMgr");
            s_instance = go.AddComponent<LuaMgr>();
            s_instance.Env = new XLua.LuaEnv();
            s_instance.Env.AddLoader(LuaMgr.LuaLoader);
            //s_instance.Env.AddBuildin("sproto.core", XLua.LuaDLL.Lua.LoadSprotoCore);
#if JDEBUG
        s_instance.Env.Global.SetInPath<int>("DEBUG", 1);
#endif
#if SDK_ENABLED
        s_instance.Env.Global.SetInPath<int>("SDK_ENABLED", 1);
#endif
#if UNITY_EDITOR
            s_instance.Env.Global.SetInPath<int>("UNITY_EDITOR", 1);
#endif
#if UNITY_IPHONE
        s_instance.Env.Global.SetInPath<int>("UNITY_IPHONE", 1);
#endif
#if UNITY_ANDROID
        s_instance.Env.Global.SetInPath<int>("UNITY_ANDROID", 1);
#endif
        }
    }

    public static void DeInit() {
        if (s_instance != null) {
            s_instance.StopAllCoroutines();
        }
    }

    public void LoadModule(string module) {
        Env.DoString(string.Format("require('{0}')", module));
    }

    public void ReloadModule(string module) {
        Env.DoString(string.Format("package.loaded['{0}'] = nil", module), "UnLoadModule");
        LoadModule(module);
    }

    // public void LoadHotFix() {
    //     var code = AssetBundleMgr.Load("Lua/HotFix.lua", "txt") as TextAsset;
    //     if(code != null) {
    //         Env.DoString(code.text, "LuaHotFix");
    //     }
    // }

    // public LuaView LoadView(GameObject parent, string fileType, string filename, string moduleName) {
    //     Log.Info("Loading lua view");
    //     GameObject prefab = null;
    //     if(ViewManager.Instance != null) {
    //         prefab = ViewManager.Instance.GetLuaViewObj(fileType);
    //     }
    //     Log.Info("Loading lua view Obj by Pool");
    //     if(prefab == null) {
    //         prefab = ResourceMgr.Load(filename, "prefab") as GameObject;
    //     }
    //     Log.Info("Loaded lua view prefab");
    //     if(prefab == null) {
    //         throw new System.IO.FileNotFoundException("Can not find filename " + filename);
    //     }
    //     Log.Info("Loading lua view prefab instance");
    //     var gameObject = Object.Instantiate(prefab) as GameObject;
    //     Log.Info("Loaded lua view prefab instance");
    //     if(gameObject == null) {
    //         throw new System.Exception("Can not instantiate prefab " + filename);
    //     }
    //     if(parent != null) {
    //         var t = gameObject.transform;
    //         t.parent = parent.transform;
    //         t.localPosition = Vector3.zero;
    //         t.localRotation = Quaternion.identity;
    //         t.localScale = Vector3.one;
    //         gameObject.layer = parent.layer;
    //     }
    //     var view = gameObject.GetComponent<LuaView>();
    //     if(view == null) {
    //         throw new System.Exception("Can not find LuaView in prefab " + filename);
    //     }
    //     var behaviour = gameObject.GetComponent<LuaBehaviour>();
    //     Log.Info("Loading lua code");
    //     InitLuaBehaviour(behaviour, moduleName);
    //     Log.Info("Loaded lua code");
    //     return view;
    // }

    public LuaBehaviour AddLuaBehaviour(GameObject gameObject, string moduleName) {
        var behaviour = gameObject.AddComponent<LuaBehaviour>();
        InitLuaBehaviour(behaviour, moduleName);
        return behaviour;
    }

    public void InitLuaBehaviour(LuaBehaviour behaviour, string moduleName) {
        XLua.LuaTable module;
        var returns = Env.DoString(string.Format("return require('{0}')", moduleName), "LoadModule");
        if (returns == null || returns.Length == 0 || (module = returns[0] as XLua.LuaTable) == null) {
            throw new XLua.LuaException(string.Format("Module {0} does not return a table", moduleName));
        }
#if UNITY_EDITOR
        Env.DoString(string.Format("package.loaded['{0}'] = nil", moduleName), "UnLoadModule");
#endif
        behaviour.SetLuaModule(module);
    }

    public LuaBehaviour AddLuaViewBehaviour(GameObject gameObject, string moduleName) {
        var behaviour = gameObject.AddComponent<LuaBehaviour>();
        InitLuaViewBehaviour(behaviour, moduleName);
        return behaviour;
    }

    public void InitLuaViewBehaviour(LuaBehaviour behaviour, string moduleName) {
        XLua.LuaTable module;
        var returns = Env.DoString(string.Format("return require('{0}')", moduleName), "LoadModule");
        if (returns == null || returns.Length == 0 || (module = returns[0] as XLua.LuaTable) == null) {
            throw new XLua.LuaException(string.Format("Module {0} does not return a table", moduleName));
        }
#if UNITY_EDITOR
        Env.DoString(string.Format("package.loaded['{0}'] = nil", moduleName), "UnLoadModule");
#endif
        behaviour.SetLuaModule(module);
    }

    public void DelayCall(float delay, XLua.LuaFunction fn) {
        StartCoroutine(DoDelayCall(delay, fn));
    }

    public void StopAllDelayCall() {
        StopAllCoroutines();
    }

    public void CallNextFrame(XLua.LuaFunction fn) {
        StartCoroutine(DoCallNextFrame(fn));
    }

    private IEnumerator DoDelayCall(float delay, XLua.LuaFunction fn) {
        yield return new WaitForSeconds(delay);
        fn.Call();
    }

    private IEnumerator DoCallNextFrame(XLua.LuaFunction fn) {
        yield return 0;
        fn.Call();
    }

    private void Update() {
        if (Time.time - m_lastGCTime > GC_INTERVAL) {
            if(Env != null) {
                Env.Tick();
            }
            m_lastGCTime = Time.time;
        }
    }

    private static byte[] LuaLoader(ref string filename) {
#if UNITY_EDITOR
        string path = filename.Replace(".", "/");
        var code = UnityEditor.AssetDatabase.LoadAssetAtPath(editorLuaPath + "/" + path + ".lua.txt", typeof(TextAsset)) as TextAsset;
        if (code != null) {
            return code.bytes;
        }
#else
    string path = filename.Replace(".", "/");
    var code = Resources.Load(targetLuacPath + "/" + path) as TextAsset;
    if(code != null) {
        byte[] decode   = new byte[code.bytes.Length];
        Joywinds.Net.RC4.Encode(code.bytes, 0, decode, 0, code.bytes.Length, System.Text.Encoding.UTF8.GetBytes(LuaMgr.password));
        return decode;
    }
#endif
        return null;
    }
}



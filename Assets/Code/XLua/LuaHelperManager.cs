// using UnityEngine;
// using XLua;
// 
// public class LuaHelperManager {
//     protected static LuaHelperManager mInstance;
//     protected LuaHelperManager() {}
//     private static GameObject parent = GameObject.Find("Canvas");
//     public static LuaHelperManager Instance {
//         get {
//             if(mInstance == null) {
//                 mInstance = new LuaHelperManager();
//             }
//             return mInstance;
//         }
//     }
// 
//     /// <summary>
//     /// Xlua层加载UI面板
//     /// </summary>
//     /// <param name="path"> 路径 </param>
//     /// <param name="OnCreate"> 创建出来的委托回调 </param>
//     public void LoadUI(string path, XLuaCustomExport.OnCreate onCreate = null) {
//         Debug.Log("加载UI窗口 =========== " + path);
//         GameObject obj =  GameObject.Instantiate(Resources.Load<GameObject>(path), parent.transform);
//         if (obj != null) {
//             obj.AddComponent<LuaViewBehaviour>();
//         }else {
//             return;
//         }
//         if (onCreate != null){
//             onCreate(obj);
//         }
//     }
// }
// 
// /// <summary>
// /// XLua的自定义拓展
// /// </summary>
// /// 
// public static class XLuaCustomExport {
//     [CSharpCallLua]
//     public delegate void OnCreate(GameObject obj);
// }
// 

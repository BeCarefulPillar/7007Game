// 加载出来的UI窗体Panel挂载上LuaViewBehaviour脚本、同时间将Unity中的脚本生命周期方法回调给Unity
// using System;
// using UnityEngine;
// using XLua;
// 
// public class LuaViewBehaviour : MonoBehaviour {
//     [CSharpCallLua]
//     public delegate void delLuaAwake(GameObject obj);
//     LuaViewBehaviour.delLuaAwake luaAwake;
// 
//     [CSharpCallLua]
//     public delegate void delLuaStart();
//     LuaViewBehaviour.delLuaStart luaStart;
// 
//     [CSharpCallLua]
//     public delegate void delLuaUpdate();
//     LuaViewBehaviour.delLuaUpdate luaUpdate;
// 
//     [CSharpCallLua]
//     public delegate void delLuaOnDestroy();
//     LuaViewBehaviour.delLuaOnDestroy luaOnDestroy;
// 
//     private LuaTable scriptEnv;
//     private LuaEnv luaEnv;
// 
//     private void Awake() {
//         //获取全局的Lua环境变量
//         luaEnv = LuaMgr.luaEnv;
// 
//         scriptEnv = luaEnv.NewTable();
// 
//         LuaTable meta = luaEnv.NewTable();
//         meta.Set("__index", luaEnv.Global);
//         scriptEnv.SetMetaTable(meta);
//         meta.Dispose();
// 
//         string prefabName = name;
//         //去掉克隆的关键字
//         if (prefabName.Contains("(Clone)")) {
//             prefabName = prefabName.Split(new string[] { "(Clone)" }, StringSplitOptions.RemoveEmptyEntries)[0];
//         }
// 
//         prefabName = prefabName.Replace("pan_", "");
// 
//         //  prefabName + ".awake"  要对应Lua脚本中View的方法
//         luaAwake = scriptEnv.GetInPath<LuaViewBehaviour.delLuaAwake>(prefabName + ".awake");
//         luaStart = scriptEnv.GetInPath<LuaViewBehaviour.delLuaStart>(prefabName + ".start");
//         luaUpdate = scriptEnv.GetInPath<LuaViewBehaviour.delLuaUpdate>(prefabName + ".update");
//         luaOnDestroy = scriptEnv.GetInPath<LuaViewBehaviour.delLuaOnDestroy>(prefabName + ".destroy");
// 
//         scriptEnv.Set("self", this);
//         if (luaAwake != null) {
//             luaAwake(this.gameObject);
//         }
//     }
// 
//     private void Start() {
// 
//         if (luaStart != null) {
//             luaStart();
//         }
//     }
// 
//     private void OnDestroy() {
//         if (luaOnDestroy != null) {
//             luaOnDestroy();
//         }
//         luaAwake = null;
//         luaOnDestroy = null;
//         luaUpdate = null;
//         luaStart = null;
// 
//     }
// }
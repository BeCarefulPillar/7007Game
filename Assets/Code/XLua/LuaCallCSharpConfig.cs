class LuaCallCSharpConfig {
    [XLua.LuaCallCSharp]
    public static System.Collections.Generic.List<System.Type> TYPES = new System.Collections.Generic.List<System.Type> {
        typeof(UnityEngine.Component),
        typeof(UnityEngine.GameObject),
        typeof(UnityEngine.Transform),
        typeof(LuaHelperManager),
    };
}

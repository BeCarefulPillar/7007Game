class CSharpCallLuaConfig {
    [XLua.CSharpCallLua]
     public static System.Collections.Generic.List<System.Type> TYPES = new System.Collections.Generic.List<System.Type>{
        typeof(System.Action),
        typeof(System.Action<XLua.LuaTable>),
        typeof(System.Action<string>),
        typeof(System.Action<bool>),
        typeof(System.Action<int>),
        typeof(System.Action<float>),
        typeof(System.Action<int, int>),
        typeof(System.Action<int, bool>),
        typeof(System.Action<string, string>),
        typeof(System.Action<string, bool>),
        typeof(System.Action<string, System.Action>),
        typeof(System.Action<float, float>),
        typeof(System.Action<float, float, float>),
        typeof(LuaBehaviour.MonoBehaviourEvent),
     };
}

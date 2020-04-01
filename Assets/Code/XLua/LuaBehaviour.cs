// Copyright (C) 2017 Joywinds Inc.

using UnityEngine;
using XLua;

public class LuaBehaviour : MonoBehaviour {
    public delegate void MonoBehaviourEvent(XLua.LuaTable luaView, XLua.LuaTable b);

    private MonoBehaviourEvent mLuaStart;
    private MonoBehaviourEvent mLuaUpdate;
    private MonoBehaviourEvent mLuaFixedUpdate;
    private MonoBehaviourEvent mLuaLateUpdate;
    private MonoBehaviourEvent mLuaOnDisable;
    private MonoBehaviourEvent mLuaOnEnable;
    private MonoBehaviourEvent mLuaOnDestroy;
    private MonoBehaviourEvent mLuaOnGUI;

    protected XLua.LuaTable mLuaModule;
    protected XLua.LuaTable mLuaObject;

    public XLua.LuaTable LuaView {
        get {
            return mLuaModule;
        }
    }
    
    public void Close() {
        Destroy(gameObject);
    }

    public void SetLuaModule(LuaTable module, LuaTable data = null) {
        mLuaModule = module;
        mLuaStart = module.Get<string, MonoBehaviourEvent>("Start");
        mLuaUpdate = module.Get<string, MonoBehaviourEvent>("Update");
        mLuaFixedUpdate = module.Get<string, MonoBehaviourEvent>("FixedUpdate");
        mLuaLateUpdate = module.Get<string, MonoBehaviourEvent>("LateUpdate");
        mLuaOnDisable = module.Get<string, MonoBehaviourEvent>("OnDisable");
        mLuaOnEnable = module.Get<string, MonoBehaviourEvent>("OnEnable");
        mLuaOnDestroy = module.Get<string, MonoBehaviourEvent>("OnDestroy");
        mLuaOnGUI = module.Get<string, MonoBehaviourEvent>("OnGUI");

        mLuaObject = LuaMgr.Instance.Env.NewTable();
        mLuaObject.Set<string, GameObject>("gameObject", gameObject);
        mLuaObject.Set<string, Transform>("transform", transform);
        mLuaObject.Set<string, LuaTable>("data", data);
        mLuaObject.Set<string, LuaBehaviour>("view", this);
    }

    public object[] Call(string funcName, params object[] args) {
        XLua.LuaFunction f = mLuaModule.Get<string, XLua.LuaFunction>(funcName);
        if (f == null) {
            throw new XLua.LuaException("No lua function: " + funcName);
        }
        object[] allArgs;
        if (args == null) {
            allArgs = new object[1];
        } else {
            allArgs = new object[args.Length + 1];
            for (int i = 0; i < args.Length; i++) {
                allArgs[i + 1] = args[i];
            }
        }
        allArgs[0] = mLuaModule;
        return f.Call(allArgs);
    }

    void Start() {
        if (mLuaStart != null) {
            mLuaStart(mLuaModule, mLuaObject);
        }
    }

    void Update() {
        if (mLuaUpdate != null) {
            UnityEngine.Profiling.Profiler.BeginSample("LuaBehaviour.Update");
            mLuaUpdate(mLuaModule, mLuaObject);
            UnityEngine.Profiling.Profiler.EndSample();
        }
    }

    void FixedUpdate() {
        if (mLuaFixedUpdate != null) {
            UnityEngine.Profiling.Profiler.BeginSample("LuaBehaviour.FixedUpdate");
            mLuaFixedUpdate(mLuaModule, mLuaObject);
            UnityEngine.Profiling.Profiler.EndSample();
        }
    }

    void LateUpdate() {
        if (mLuaLateUpdate != null) {
            UnityEngine.Profiling.Profiler.BeginSample("LuaBehaviour.LateUpdate");
            mLuaLateUpdate(mLuaModule, mLuaObject);
            UnityEngine.Profiling.Profiler.EndSample();
        }
    }

    void OnDisable() {
        if (mLuaOnDisable != null) {
            mLuaOnDisable(mLuaModule, mLuaObject);
        }
    }

    void OnEnable() {
        if (mLuaOnEnable != null) {
            mLuaOnEnable(mLuaModule, mLuaObject);
        }
    }

    void OnDestroy() {
        if (mLuaOnDestroy != null) {
            mLuaOnDestroy(mLuaModule, mLuaObject);
        }
        if (mLuaObject != null) {
            mLuaObject.Dispose();
            mLuaObject = null;
        }
        mLuaModule = null;
        mLuaUpdate = null;
        mLuaOnDestroy = null;
        mLuaStart = null;
    }

    void OnGUI() {
        if (mLuaOnGUI != null) {
            mLuaOnGUI(mLuaModule, mLuaObject);
        }
    }
}


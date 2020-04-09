using UnityEditor;
using UnityEngine;



public class MyFirstWindow : EditorWindow
{
    string bugReporterName = "";
    string description = "";
    GameObject buggyGameObject;

    MyFirstWindow() {
        this.titleContent = new GUIContent("Bug reporter");
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    [MenuItem("Tool/Bug Reporter")]
    static void showWindow() {
        EditorWindow.GetWindow(typeof(MyFirstWindow));
    }
}

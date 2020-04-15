using System.Collections.Generic;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

public class MyWindow : EditorWindow
{
    static MyWindow myWindow;
    static Scene scene;
    List<Rect> rectList;

    [MenuItem("Tool/Bug Reporter")]
    static void Init() {
        myWindow = (MyWindow)EditorWindow.GetWindow(typeof(MyWindow), false, "MyWindow", true);//创建窗口
        myWindow.Show();
        scene = EditorSceneManager.OpenScene("Assets/Scene/edit.unity");
        myWindow.rectList = new List<Rect>();
    }

    private void OnGUI() {
        EditorGUILayout.LabelField(EditorWindow.focusedWindow.ToString());
        if (GUILayout.Button("创建Rect")) {
           Debug.DrawLine(new Vector3(0,0,0), new Vector3(100,0,0),Color.red);
           //rectList.Add(new Rect(0,0,5,5));
        }
        if (GUILayout.Button("关闭窗口")) {
            EditorSceneManager.OpenScene("Assets/Scene/game_test.unity");
            myWindow.Close();
        }
    }

    void OnDrawGizmos() {
        foreach (var rect in rectList) {
            //DrawHelper.DrawRect(rect);
        }
        
    }
}

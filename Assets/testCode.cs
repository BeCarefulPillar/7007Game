using UnityEngine;
using System.Collections;
using Spine.Unity;

public class testCode : MonoBehaviour {
    private GameObject heroObj;
    private SkeletonAnimation skeletonAnimation;
    private MeshRenderer mesh;
    void Start() {
        heroObj = GameObject.Instantiate(Resources.Load<GameObject>("Prefab/Soldier/Clark/Clark0"), transform.parent);
        Debug.Log(heroObj.transform.position);
        foreach (Transform child in  heroObj.transform.GetComponentsInChildren<Transform>()) {
            mesh = child.GetComponent<MeshRenderer>();
            if (mesh != null) {
                Debug.Log(mesh.bounds.size);
            }
        }
        
        
    }
    
    void LateUpdate() {
        if (mesh != null) {
            DrawHelper.DrawRect(new Rect(heroObj.transform.position - mesh.bounds.size/2, mesh.bounds.size), transform.parent.position.z,  Color.red);
        }

        var h = Input.GetAxis("Horizontal");
        Debug.Log(h);

        heroObj.transform.position = new Vector3(heroObj.transform.position.x + 3 * Time.deltaTime, heroObj.transform.position.y,heroObj.transform.position.z) ;
    }

}

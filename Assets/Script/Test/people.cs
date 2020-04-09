using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[AddComponentMenu("Learning/People")]
[RequireComponent(typeof(Rigidbody))]
public class people : MonoBehaviour
{
    public string name;
    public int age;
    
    [ContextMenu("OutputInfo")]
    void OutputInfo() {
        print(name + "|" + age);
    }
    
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}

Shader "Test/ff1"
{   
    Properties {
        _Color("Main Color", color) = (1,0,0,1)
        _Ambient("Ambient", color) = (0.3, 0.3, 0.3, 0.3)
        _Specular("Specular", color) = (1,1,1,1)
        _Shininess("Shininess", range(0, 8)) = 4
        _Emissioin("Emission", color) =(1,1,1,1)

        _Constant("ConstantColor", color) = (1,1,1,0.5) //常数
        _MainTex("MainTex", 2d) = ""{}
        _SecondTex("SecondTex", 2d) = ""{}
    }
    SubShader {
        Tags{"Queue" = "Transparent"}

        Pass {
            Blend Srcalpha OneMinusSrcAlpha
            //color(1,1,1,1)
            Material {
                diffuse[_Color]  //漫反射
                ambient[_Ambient] //环境光
                specular[_Specular] //镜面反射
                shininess[_Shininess] //高光
                emission[_Emission] //自发光
            }

            lighting on
            separatespecular on  //镜面反射开关

            SetTexture[_MainTex]{
                combine  texture  * primary double  
            }

            SetTexture[_SecondTex]{
                constantColor[_Constant]
                combine texture * previous double, texture * constant   //借用texture alpha 通道
            }
        }
    }
}

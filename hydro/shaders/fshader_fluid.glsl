#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

//принимаем координаты камеры
uniform vec3 u_camera;
//принимаем координаты источника света
uniform vec3 u_lightPosition;
//принимаем координаты пикселя для поверности после интерполяции
varying vec3 v_vertex;
//принимаем вектор нормали для пикселя после интерполяции
varying vec3 v_normal;
//принимаем цвет пикселя после интерполяции
varying vec4 v_color;


//! [0]
void main()
{
    //u_camera = vec3( 100.0, 0.0, 0.0 );
    //u_lightPosition = vec3( 0.0, 0.0, 0.0 );

    //повторно нормализуем нормаль пикселя,
    //т.к. при интерполяции нормализация может нарушиться
    vec3 n_normal = normalize(v_normal);
    //вычисляем единичный вектор, указывающий из пикселя на источник света
    vec3 lightvector = normalize(u_lightPosition - v_vertex);
    //вычисляем единичный вектор, указывающий из пикселя на камеру
    vec3 lookvector = normalize(u_camera - v_vertex);
    //определяем яркость фонового освещения
    float ambient = 0.2;
    //определяем коэффициент диффузного освещения
    float k_diffuse = 0.8;
    //определяем коэффициент зеркального освещения
    float k_specular = 0.4;
    //вычисляем яркость диффузного освещения пикселя
    float diffuse = k_diffuse * max(dot(n_normal, lightvector), 0.0);
    //вычисляем вектор отраженного луча света
    vec3 reflectvector = reflect(-lightvector, n_normal);
    //вычисляем яркость зеркального освещения пикселя
    float specular = k_specular * pow( max(dot(lookvector,reflectvector),0.0), 40.0 );
    //определяем вектор белого цвета
    vec4 one = vec4(1.0,1.0,1.0,1.0);
    //вычисляем цвет пикселя
    vec4 lightColor = (ambient+diffuse+specular)*one;
    //и смешаем его наполовину с цветом пикселя:
    gl_FragColor = mix(lightColor, v_color, 0.4);
    //gl_FragColor = v_color;
}
//! [0]


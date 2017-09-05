#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

uniform vec2 resolution;

//uniform sampler2D texture;

//varying vec2 v_texcoord;
//uniform int points_length;
//uniform vec2 points[200];

vec2 position;

#define Thickness 0.003


/*float drawLine(vec2 p1, vec2 p2) {
  vec2 uv = gl_FragCoord.xy / resolution;

  float a = abs(distance(p1, uv));
  float b = abs(distance(p2, uv));
  float c = abs(distance(p1, p2));

  if ( a >= c || b >=  c ) return 0.0;

  float p = (a + b + c) * 0.5;

  // median to (p1, p2) vector
  float h = 2.0 / c * sqrt( p * ( p - a) * ( p - b) * ( p - c));

  return mix(1.0, 0.0, smoothstep(0.5 * Thickness, 1.5 * Thickness, h));
}*/

float line(vec2 p1, vec2 p2, float thickness) {

    /*float a = abs(distance(p1, position.xy));
    float b = abs(distance(p2, position.xy));
    float c = abs(distance(p1, p2)) + 1.0;*/

    float a = (p1.x - position.x) * (p1.x - position.x) + (p1.y - position.y) * (p1.y - position.y);
    float b = (p2.x - position.x) * (p2.x - position.x) + (p2.y - position.y) * (p2.y - position.y);
    float c = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);

    if ( a >= c || b >=  c ) return 0.0;

    float A = p1.y - p2.y;
    float B = p2.x - p1.x;
    float C = p1.x * p2.y - p2.x * p1.y;
    float D = abs(A * position.x + B * position.y + C);

    float d = D / sqrt(A * A + B * B);

    if ( d <= thickness || D == 0.0 ) {
        return mix(thickness, 0.0, smoothstep(0.0, thickness, d));
    } else {
        return 0.0;
    }
}

//принимаем координаты камеры
uniform vec3 u_camera;
//принимаем координаты источника света
uniform vec3 u_lightPosition;
//принимаем координаты пикселя для поверности после интерполяции
varying vec4 v_vertex;
//принимаем вектор нормали для пикселя после интерполяции
varying vec3 v_normal;
//принимаем цвет пикселя после интерполяции
varying vec4 v_color;


//! [0]
void main()
{
    position = vec2(gl_FragCoord.x, resolution.y - gl_FragCoord.y);
    vec4 color;

    color = vec4(0.0, 1.0, 0.0, 1.0);

    float f = 0.0, r = 0.0;

    /*for (int i = 0; i < points_length - 1; ++i) {
        r = line(points[i], points[i+1], 2.0);
        if (r > f) {
            f = r;
        }
    }*/
    /*r = line(points[0], points[1], 2.0);
    if (r > f) {
        f = r;
    }
    r = line(points[1], points[2], 2.0);
    if (r > f) {
        f = r;
    }
    r = line(points[2], points[3], 2.0);
    if (r > f) {
        f = r;
    }
    r = line(points[4], points[5], 2.0);
    if (r > f) {
        f = r;
    }*/

    //gl_FragColor = vec4( 1.0, 0, 0, f );
    //gl_FragColor = vec4( 1.0, 0, 0, 1.0 );
    //gl_FragColor = gl_Color;

    u_camera = vec3( 100.0, 0.0, 0.0 );
    u_lightPosition = vec3( 0.0, 0.0, 0.0 );

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
    gl_FragColor = gl_Color;
}
//! [0]


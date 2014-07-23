#version 420

in vec3 position, normal, position_eye, normal_eye;
in vec2 texcoord;
in vec4 shadow_coord;

//layout (binding = 1) uniform sampler2D disp_tex;

uniform mat4 view_mat;
uniform vec3 sun_direction;
uniform sampler2D shadow_map;
uniform int water;
uniform float time;

// sunlight properties
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour

out vec4 frag_color; // final colour of surface

float dist = distance(vec3(0.0), position_eye);

//placeholder noise equations. please ignore
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  // x0 = x0 - 0.0 + 0.0 * C.xxx;
  // x1 = x0 - i1 + 1.0 * C.xxx;
  // x2 = x0 - i2 + 2.0 * C.xxx;
  // x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ ); // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }


float eval_shadow () {
//	return 1;
	vec2 poisson_disk[4] = vec2[](
	  vec2( -0.94201624, -0.39906216 ),
	  vec2( 0.94558609, -0.76890725 ),
	  vec2( -0.094184101, -0.92938870 ),
	  vec2( 0.34495938, 0.29387760 )
	);

	const float epsilon = 0.003;
	
	if(sun_direction.z < 0) return 0;

	float factor = 1;
	for (int i=0;i<4;i++) {
	  if (texture(shadow_map, shadow_coord.xy + poisson_disk[i]/1000).z  <  shadow_coord.z-epsilon){
		factor-=0.2;
	  }
	}
	return factor;
}


vec4 terrain_color() {
  
	// surface reflectance
	vec3 Kd = vec3 (0.2, 0.2, 0.2); // grey diffuse surface reflectance
	vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light

	// surface colors
	vec3 green = vec3(0.2, 0.4, 0.22);
	//vec3 green = vec3(0, 0, 1);

	vec3 brown = vec3(0.35, 0.35, 0.24);
	//vec3 brown = vec3(1, 0, 0);

	vec3 gray = vec3(0.35, 0.35, 0.35);
	
	//initial terrain coloring
	float slope = max(dot(normal, vec3(0, 0, 1)), 0.0);
	float t1 = smoothstep(0.8, 0.9, slope);
	float t2 = smoothstep(0.7, 0.8, slope);
	Kd = mix(gray, mix(brown, green, t1), t2);

	if(position.z < 5) {
		Kd = mix(vec3(0.5, 0.6, 0.4), Kd, smoothstep(0, 5.0, position.z));
	}

	if(position.z < 0) {
		//TODO: replace conceptual smoothstep functions with something that's actually reasonably fast
		Kd = mix(vec3(0.2, 0.4, 0.5), Kd, smoothstep(-2, 0, position.z));
		Kd = mix(vec3(0.2, 0.4, 0.4), Kd, smoothstep(-6, 0, position.z));
	}

	Ka = Kd * 0.3;

	// ambient intensity
	vec3 Ia = La * Ka;
	
	float dot_prod = max(dot(sun_direction, normal), 0.0);
	
	// diffuse intensity
//	vec3 light_position_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	vec3 Id = Ld * Kd * dot_prod; //final intensity
  
	// shaded color
	return vec4 (Id * (0.5 + 0.5 * eval_shadow()) + Ia, 1.0);
}


vec4 water_color() {
  
	// surface reflectance
	vec3 Kd = vec3 (0.2, 0.2, 0.2); // grey diffuse surface reflectance
	vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light

	// ambient intensity
	vec3 Ia = vec3(0.1, 0.3, 0.9) * 0.4;

	// diffuse intensity
	vec3 direction_to_light_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	float dot_prod = dot(direction_to_light_eye, normal_eye);
	dot_prod = max(dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; //final intensity

	int specular_exponent = 10;
  
	// specular intensity
	float noise = 0;
	noise = snoise(vec3(position.x, position.y, time * 0.5))* 1 * (dist)/200;

	//TODO: more realistic perturbations
	vec3 surface_to_viewer_eye = normalize(-position_eye - noise * 2);
	vec3 half_angle = normalize(direction_to_light_eye + surface_to_viewer_eye);
	float dot_prod_specular = dot(normal_eye, half_angle);
	dot_prod_specular = clamp(dot_prod_specular, 0.0, 1.0);
	float specular_factor = pow(dot_prod_specular, specular_exponent);
	vec3 Is = vec3(specular_factor);

	//if sun is below horizon don't highlight
	if(sun_direction.z < 0) Is *= 0;

	float alpha = min(1.0, 1 - max(0.0, abs(dot(normalize(position_eye), normal_eye))) + 0.1);
  
	// final colour
	return vec4 ((Is + Id + Ia) * (0.5 + 0.5 * eval_shadow()), alpha);
}


void main () {

	// day-night cycle factor
	float cycle = dot(sun_direction, vec3(0.0, 0.0, 1.0));
	float cycle_factor = smoothstep(-0.5, 0.1, cycle);

	vec4 pre_color = vec4(0.0);

	if(water == 0) pre_color = terrain_color();
	else if(water == 1) pre_color = water_color();

	vec4 fog_color = vec4(mix(vec3(0.1, 0.1, 0.1), vec3(0.5, 0.6, 0.75), cycle_factor), 1.0f);

	const float min_fog_radius = 100.0;
	const float max_fog_radius = 800.0;

	// work out distance from camera to point
	float dist = length (-position_eye);
	// get a fog factor (thickness of fog) based on the distance
	float fog_fac = (dist - min_fog_radius) / (max_fog_radius - min_fog_radius);
	// constrain the fog factor between 0 and 1
	fog_fac = clamp (fog_fac, 0.0, 1.0);

	frag_color = mix (pre_color, fog_color, fog_fac);

//	frag_color = vec4(position.z/1, 0.0, 0.0, 1.0);
}
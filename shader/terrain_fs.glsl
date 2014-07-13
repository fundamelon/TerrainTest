#version 420

in vec3 normal, position_eye, normal_eye;
in vec4 shadow_coord;

uniform mat4 view_mat;

uniform vec3 sun_direction;

uniform sampler2D shadow_map;

// sunlight properties
vec3 Ld = vec3 (0.4, 0.4, 0.4); // dull white diffuse light colour
vec3 La = vec3 (0.3, 0.3, 0.3); // grey ambient colour
  
// surface reflectance
vec3 Kd = vec3 (0.2, 0.2, 0.2); // grey diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light

// surface colors
vec3 green = vec3(0.2, 0.5, 0.12);
//vec3 green = vec3(0, 0, 1);

vec3 brown = vec3(0.35, 0.35, 0.24);
//vec3 brown = vec3(1, 0, 0);

vec3 gray = vec3(0.4, 0.4, 0.4);

out vec4 frag_color; // final colour of surface


const float epsilon = 0.0042;

vec2 poisson_disk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float eval_shadow () {
	
	if(sun_direction.z < 0) return 0;

	float factor = 1;
	for (int i=0;i<4;i++) {
	  if (texture(shadow_map, shadow_coord.xy + poisson_disk[i]/1000).z  <  shadow_coord.z-epsilon){
		factor-=0.25;
	  }
	}
	return factor;
}


void main () {
	//initial terrain coloring
	float slope = max(dot(normal, vec3(0, 0, 1)), 0.0);
	float t1 = smoothstep(0.7, 0.9, slope);
	float t2 = smoothstep(0.5, 0.7, slope);
	Kd = mix(gray, mix(brown, green, t1), t2);
	Ka = Kd * 0.1;

	vec3 Ia = vec3(0.3, 0.3, 0.3);

	// ambient intensity
	Ia *= La * Ka;
	
	// diffuse intensity
	vec3 light_position_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	float dot_prod = max(dot(sun_direction, normal), 0.0);
	vec3 Id = Ld * Kd * dot_prod; //final intensity
  
	// shaded color
	vec4 shaded_color = vec4 (Id * eval_shadow() + Ia, 1.0);

	 // fog variables
	 
	//day/night cycle factor
	float cycle = dot(sun_direction, vec3(0.0, 0.0, 1.0));
	float cycle_factor = smoothstep(-0.5, 0.1, cycle);
	
	vec4 fog_color = vec4(mix(vec3(0.1, 0.1, 0.1), vec3(0.6, 0.7, 0.8), cycle_factor), 1.0f);

	const float min_fog_radius = 0.0;
	const float max_fog_radius = 60.0;

	// work out distance from camera to point
	float dist = length (-position_eye);
	// get a fog factor (thickness of fog) based on the distance
	float fog_fac = (dist - min_fog_radius) / (max_fog_radius - min_fog_radius);
	// constrain the fog factor between 0 and 1
	fog_fac = clamp (fog_fac, 0.0, 1.0);

	// blend the fog colour with the lighting colour, based on the fog factor
	frag_color = mix (shaded_color, fog_color, fog_fac);
//	frag_color = shaded_color * vec4(1.0, 1.0, 1.0, 1-fog_fac);
}
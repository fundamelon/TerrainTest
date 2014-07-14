#version 420

in vec3 position, normal, position_eye, normal_eye;
in vec4 shadow_coord;

uniform mat4 view_mat;
uniform vec3 sun_direction;
uniform sampler2D shadow_map;
uniform int water;
uniform float time;

// sunlight properties
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
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

float dot_prod;

const float epsilon = 0.003;

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
		factor-=0.2;
	  }
	}
	return factor;
}


void terrain_shading() {
	
	//initial terrain coloring
	float slope = max(dot(normal, vec3(0, 0, 1)), 0.0);
	float t1 = smoothstep(0.8, 0.9, slope);
	float t2 = smoothstep(0.7, 0.8, slope);
	Kd = mix(gray, mix(brown, green, t1), t2);
	if(position.z < 0) {
		//TODO: replace conceptual smoothstep functions with something that's actually reasonably fast
		Kd = mix(vec3(0.2, 0.4, 0.5), Kd, smoothstep(-0.1, 0, position.z));
		Kd = mix(vec3(0.2, 0.4, 0.4), Kd, smoothstep(-5, 0, position.z));
	}
	Ka = Kd * 0.1;

	vec3 Ia = vec3(0.3, 0.3, 0.3);

	// ambient intensity
	Ia *= La * Ka;
	
	// diffuse intensity
	vec3 light_position_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	vec3 Id = Ld * Kd * dot_prod; //final intensity
  
	// shaded color
	vec4 shaded_color = vec4 (Id * eval_shadow() + Ia, 1.0);

	 // fog variables
	 
	//day/night cycle factor
	float cycle = dot(sun_direction, vec3(0.0, 0.0, 1.0));
	float cycle_factor = smoothstep(-0.5, 0.1, cycle);
	
	vec4 fog_color = vec4(mix(vec3(0.1, 0.1, 0.1), vec3(0.6, 0.7, 0.8), cycle_factor), 1.0f);

	const float min_fog_radius = 50.0;
	const float max_fog_radius = 4000.0;

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


void water_shading() {
	// ambient intensity
	vec3 Ia = vec3(0.1, 0.3, 0.9) * 0.4;

	// diffuse intensity
	vec3 direction_to_light_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	float dot_prod = dot(direction_to_light_eye, normal_eye);
	dot_prod = max(dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; //final intensity

	int specular_exponent = 4;
  
	// specular intensity
	vec3 reflection_eye = reflect(-direction_to_light_eye, normal_eye);
	vec3 surface_to_viewer_eye = normalize(-position_eye);
	float dot_prod_specular = dot(reflection_eye, surface_to_viewer_eye);
	dot_prod_specular = max(dot_prod_specular, 0.0);
	float specular_factor = pow(dot_prod_specular, specular_exponent);
	vec3 Is = vec3(specular_factor); //final intensity
  
	// final colour
	frag_color = vec4 ((Is + Id + Ia) * (0.5 + 0.5 * eval_shadow()), min(1.0, 1 - max(0.0, -dot(normalize(position_eye), normal_eye)) + 0.1));
//	frag_color = vec4(0.0, 0.0, 1.0 * eval_shadow(), 1-max(0.0, -dot(normalize(position_eye), normal_eye)));
}


void main () {
	dot_prod = max(dot(sun_direction, normal), 0.0);

	if(water == 0) terrain_shading();
	else if(water == 1) water_shading();
}
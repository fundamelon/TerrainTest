#version 420

in vec3 position_world;

uniform vec3 sun_direction;

out vec4 frag_color;

vec3 sun_color = vec3(1.0, 1.0, 0.9);

vec3 horizonDayColor = vec3(0.6, 0.7, 0.8);
vec3 zenithDayColor = vec3(0.3, 0.4, 1.0);

vec3 horizonNightColor = vec3(0.1, 0.1, 0.1);
vec3 zenithNightColor = vec3(0, 0, 0.1);


void main() {
	vec3 dir = normalize(position_world);

	vec3 sky_color;

	//day/night cycle factor
	float cycle = dot(sun_direction, vec3(0.0, 0.0, 1.0));
	float cycle_factor = smoothstep(-0.5, 0.1, cycle);

	//distance of eye ray to sun dir
	float dist_from_sun = distance(sun_direction, dir);

	//pick range for sun
	float t = smoothstep(0.03, 0.05, dist_from_sun);

	//sky color from elevation
	float sky_factor = max(0.0, dot(dir, vec3(0.0, 0.0, 1.0)));

	vec3 horizonColor = mix(horizonNightColor, horizonDayColor, cycle_factor);
	vec3 zenithColor = mix(zenithNightColor, zenithDayColor, cycle_factor);

	sky_color = mix(horizonColor, zenithColor, sky_factor);

	sky_color += pow(max(0.0, 1-dist_from_sun), 3) * 0.3f;

	//add sun circle (temporary)
	sky_color = mix(sun_color, sky_color, t);

	frag_color = vec4(sky_color, 1);
}
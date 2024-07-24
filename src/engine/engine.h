#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <iomanip>
using namespace std;
float getDeltaTime() {
    static float lastFrame = 0.0f;
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return deltaTime;
}
float deltaTime=getDeltaTime();
struct Vector3{
	Vector3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		vec[0]=&x;
		vec[1]=&y;
		vec[2]=&z;
	}
	Vector3(float _x,float _y,float _z){
		x=_x;
		y=_y;
		z=_z;
		vec[0]=&x;
		vec[1]=&y;
		vec[2]=&z;
	}
	bool operator==(const Vector3& _vector) const
	{
		return (this->x==_vector.x&&this->y==_vector.y&&this->z==_vector.z);
	}
	bool operator!=(const Vector3& _vector) const
	{
		return !(this->x==_vector.x&&this->y==_vector.y&&this->z==_vector.z);
	}
	Vector3 operator+(const Vector3& _vector) const
	{
		return Vector3(this->x+_vector.x,this->y+_vector.y,this->z+_vector.z);
	}
	Vector3 operator-(const Vector3& _vector) const
	{
		return Vector3(this->x-_vector.x,this->y-_vector.y,this->z-_vector.z);
	}
	Vector3 operator-(const float& num) const
	{
		return Vector3(this->x-num,this->y-num,this->z-num);
	}
	Vector3 operator*(const float& num) const
	{
		return Vector3(this->x*num,this->y*num,this->z*num);
	}
	float x;
	float y;
	float z;
	float* vec[3];
};
struct Color4{
	Color4(float _r, float _g, float _b, float _a){
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	float r;
	float g;
	float b;
	float a;
};
struct Mesh
{
	Mesh(){
		vertices = {
			Vector3(-1, -1, -1),
			Vector3(1, -1, -1),
			Vector3(1, 1, -1),
			Vector3(-1, 1, -1),
			Vector3(-1, -1, 1),
			Vector3(1, -1, 1),
			Vector3(1, 1, 1),
			Vector3(-1, 1, 1)
		};
		faces = {
			{0, 1, 2, 3},
			{4, 5, 6, 7},
			{0, 1, 5, 4},
			{2, 3, 7, 6},
			{0, 3, 7, 4},
			{1, 2, 6, 5}
		};
		colors = {
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1),
			Vector3(1, 1, 1)
		};
	}
	Mesh(vector<Vector3>& _vertices,vector<vector<unsigned int>>& _faces){
		vertices = _vertices;
		faces = _faces;
	}
	vector<Vector3> vertices;
	vector<vector<unsigned int>> faces;
	vector<Vector3> colors;
};
string vector3to_string(Vector3 vect){
	return to_string(vect.x)+","+to_string(vect.y)+","+to_string(vect.z);
}
glm::vec3 vector3to_glm(Vector3 vect){
	return glm::vec3(vect.x,vect.y,vect.z);
}
Vector3 apply_vertex_rotation(Vector3 vertex, Vector3 rotation){
	Vector3 _vertex = vertex;
	Vector3 rotation_sin;
	Vector3 rotation_cos;
	rotation_sin.x = sin(rotation.x);
	rotation_sin.y = sin(rotation.y);
	rotation_sin.z = sin(rotation.z);
	rotation_cos.x = cos(rotation.x);
	rotation_cos.y = cos(rotation.y);
	rotation_cos.z = cos(rotation.z);
	_vertex.y = vertex.y * rotation_cos.x - vertex.z * rotation_sin.x;
	_vertex.z = vertex.z * rotation_cos.x + vertex.y * rotation_sin.x;
	vertex = _vertex;
	_vertex.x = vertex.x * rotation_cos.y + vertex.z * rotation_sin.y;
	_vertex.z = vertex.z * rotation_cos.y - vertex.x * rotation_sin.y;
	vertex = _vertex;
	_vertex.x = vertex.x * rotation_cos.z - vertex.y * rotation_sin.z;
	_vertex.y = vertex.y * rotation_cos.z + vertex.x * rotation_sin.z;
	return _vertex;
}
double point_distance(Vector3 point0, Vector3 point1){
	double distance = 0;
	double dx = point1.x-point0.x;
	double dy = point1.y-point0.y;
	double dz = point1.z-point0.z;
	dx = pow(dx,2);
	dy = pow(dy,2);
	dz = pow(dz,2);
	distance = dx+dy+dz;
	distance = sqrt(distance);
	return distance;
}
float sigmoid(float number) {
	return 1 / (1 + exp(-number));
}
Vector3 cross_vectors(Vector3 vec0, Vector3 vec1){
	Vector3 result;
	result.x = vec0.y*vec1.z-vec0.z*vec1.y;
	result.y = vec0.z*vec1.x-vec0.x*vec1.z;
	result.z = vec0.x*vec1.y-vec0.y*vec1.x;
	return result;
}
char* string_to_char(string str,char* out){
	for(int i=0;i<128;i++){
		out[i] = str[i];
	}
	return out;
}
vector<string> split_string(string str, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;
	while ((pos_end = str.find(delimiter, pos_start)) != string::npos) {
		token = str.substr (pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back (token);
	}
	res.push_back (str.substr (pos_start));
	return res;
}
float fov = 80.0f;
auto setup_matrix = [](GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
};
class Camera{
	public:
		char name[2][128];
		Vector3 position = Vector3(0,0,0);
		Vector3 rotation = Vector3(0,1.57,0);;
		float yaw = 0;
	Camera() : name() {
		string_to_char("none",name[0]);
	}
};
Camera* starting_camera = new Camera;
bool mesh_culling = true;
bool mesh_lighting = true;
float lighting_severity = .7;
GLFWwindow* window;
bool pre_logic_ran = false;
vector<reference_wrapper<Camera>> scene_cameras;
reference_wrapper<Camera> current_camera = *starting_camera;
class Object{
	public:
		char name[2][128];
		Vector3 position;
		Vector3 rotation;
		Vector3 scale = Vector3(.125,.125,.125);
		Mesh mesh;
		bool wireframe = false;
	Object() : name() {
		string_to_char("none",name[0]);
	}
	void from_file(string path){
		if(path.size()<3){return;}
		string path_substring = path.substr(path.size()-3,3);
		transform(path_substring.begin(), path_substring.end(), path_substring.begin(), ::tolower);
		if (path_substring.compare("obj")==0){
			ifstream file(path);
			if (!file.is_open()){
				return;
			}
			mesh.vertices.clear();
			mesh.faces.clear();
			mesh.colors.clear();
			string current_line;
			vector<Vector3> vertices;
			vector<vector<unsigned int>> faces;
			vector<Vector3> colors;
			while (getline(file, current_line)){
				vector<string> split;
				if (current_line.substr(0,2).compare("v ") == 0){
					split = split_string(current_line.substr(2)," ");
					Vector3 vertex(stof(split[0]),stof(split[1]),stof(split[2]));
					vertices.push_back(vertex);
					colors.push_back(Vector3(((float)rand()/RAND_MAX),((float)rand()/RAND_MAX),((float)rand()/RAND_MAX)));
				}
				if (current_line.substr(0,2).compare("f ") == 0){
					split = split_string(current_line.substr(2)," ");
					vector<string> _split;
					vector<unsigned int> face;
					for(int i=0;i<split.size();i++){
						_split = split_string(split[i],"/");
						face.push_back(stoi(_split[0])-1);
					}
					faces.push_back(face);
					face.clear();
					_split.clear();
				}
				split.clear();
			}
			mesh.vertices = vertices;
			vertices.clear();
			mesh.faces = faces;
			faces.clear();
			mesh.colors = colors;
			colors.clear();
			file.close();
		}
	}
	void from_mesh(Mesh new_mesh){
		mesh.vertices.clear();
		mesh.faces.clear();
		mesh.faces = new_mesh.faces;
		mesh.vertices = new_mesh.vertices;
	}
	void draw_mesh(){
		glColor3f(1.0f, 1.0f, 1.0f);
		for(int i=0;i<mesh.faces.size();i++){
			bool actually_render = false;
			if(mesh_culling){
				for(int _i=0;_i<mesh.faces[i].size();_i++){
					Vector3 vertex = mesh.vertices[mesh.faces[i][_i]];
					vertex.x*=scale.x;
					vertex.y*=scale.y;
					vertex.z*=scale.z;
					vertex = apply_vertex_rotation(vertex,rotation);
					vertex = vertex+position;
					double distance_from_camera = point_distance(current_camera.get().position, vertex);
					float distance_factor = sigmoid(distance_from_camera)*(distance_from_camera*lighting_severity);
					if(distance_factor<=1){
						actually_render = true;
						break;
					}
				}
			}
			if(actually_render||!mesh_culling){
				if(wireframe){
					glBegin(GL_LINE_LOOP);
				}else{
					glBegin(GL_POLYGON);
				}
				for(int _i=0;_i<mesh.faces[i].size();_i++){
					Vector3 vertex = mesh.vertices[mesh.faces[i][_i]];
					vertex.x*=scale.x;
					vertex.y*=scale.y;
					vertex.z*=scale.z;
					vertex = apply_vertex_rotation(vertex,rotation);
					vertex = vertex+position;
					double distance_from_camera = point_distance(current_camera.get().position, vertex);
					float distance_factor = sigmoid(distance_from_camera)*(distance_from_camera*lighting_severity);
					Vector3 vertex_color = mesh.colors[mesh.faces[i][_i]];
					if(mesh_lighting){
						vertex_color = vertex_color-distance_factor;
						if(vertex_color.x<0)
							vertex_color.x=0;
						if(vertex_color.y<0)
							vertex_color.y=0;
						if(vertex_color.z<0)
							vertex_color.z=0;
					}
					glColor3f(vertex_color.x, vertex_color.y, vertex_color.z);
					glVertex3f(vertex.x, vertex.y, vertex.z);
				}
				glEnd();
			}
		}
	}
};
bool cursor_lock = false;
bool hide_cursor = true;
uint_fast16_t default_cursor_pos[2];
uint_fast16_t changed_cursor_pos[2];
vector<reference_wrapper<Object>> scene_objects;

void built_in_movement(float move_speed,float turn_speed){
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		current_camera.get().position.y-=move_speed*deltaTime;
	if(glfwGetKey(window, GLFW_KEY_SPACE))
		current_camera.get().position.y+=move_speed*deltaTime;
	if(glfwGetKey(window,GLFW_KEY_W)){
		Vector3 position_offset = apply_vertex_rotation(Vector3(move_speed*deltaTime,0,0),current_camera.get().rotation);
		current_camera.get().position = current_camera.get().position+position_offset;
	}
	if(glfwGetKey(window,GLFW_KEY_A)){
		Vector3 position_offset = apply_vertex_rotation(Vector3(0,0,-move_speed*deltaTime),current_camera.get().rotation);
		current_camera.get().position = current_camera.get().position+position_offset;
	}
	if(glfwGetKey(window,GLFW_KEY_S)){
		Vector3 position_offset = apply_vertex_rotation(Vector3(-move_speed*deltaTime,0,0),current_camera.get().rotation);
		current_camera.get().position = current_camera.get().position+position_offset;
	}
	if(glfwGetKey(window,GLFW_KEY_D)){
		Vector3 position_offset = apply_vertex_rotation(Vector3(0,0,move_speed*deltaTime),current_camera.get().rotation);
		current_camera.get().position = current_camera.get().position+position_offset;
	}
	if(cursor_lock&&(default_cursor_pos[0]!=changed_cursor_pos[0]||default_cursor_pos[1]!=changed_cursor_pos[1])){ // Check cursor lock and if position changed
		int_fast16_t mouse_movement[2] = {
			static_cast<int_fast16_t>(abs(default_cursor_pos[0]-changed_cursor_pos[0])), // Cast so compiler sees int to int16 is delibrate so that it won't show warnings
			static_cast<int_fast16_t>(abs(default_cursor_pos[1]-changed_cursor_pos[1]))
		};
		if(mouse_movement[0]>0){
			if(default_cursor_pos[0]<changed_cursor_pos[0])
				mouse_movement[0]=-mouse_movement[0];
			current_camera.get().rotation.y+=turn_speed*mouse_movement[0];
		}
		if(mouse_movement[1]>0){
			if(default_cursor_pos[1]<changed_cursor_pos[1])
				mouse_movement[1]=-mouse_movement[1];
			current_camera.get().yaw+=turn_speed*mouse_movement[1];
			if(current_camera.get().yaw>2)
				current_camera.get().yaw = 2;
			if(current_camera.get().yaw<-2)
				current_camera.get().yaw = -2;
		}
	}
}
void updateTitleWithFPS(GLFWwindow* window,string original_title, double& lastTime, int& nbFrames) {
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0) {
        double fps = double(nbFrames) / (currentTime - lastTime);
        ostringstream ss;
        ss << fixed << setprecision(1) << fps;
    	string fpsString = original_title + " FPS: " + ss.str();
        glfwSetWindowTitle(window, fpsString.c_str());
        nbFrames = 0;
        lastTime += 1.0;
    }
}
Vector3 vector3_from_string(string str){
	vector<string> vec = split_string(str,",");
	Vector3 ret_value = Vector3(stof(vec[0]),stof(vec[1]),stof(vec[2]));
	vec.clear();
	return ret_value;
}
void load_scene(string path){
	if(path.size()<5){return;}
	string path_substring = path.substr(path.size()-5,5);
	transform(path_substring.begin(), path_substring.end(), path_substring.begin(), ::tolower);
	if (path_substring.compare("scene")==0){
		ifstream file(path);
		if (!file.is_open()){
			return;
		}
		for(int i=0;i<scene_objects.size();i++){
			delete &scene_objects[i].get();
		}scene_objects.clear();
		for(int i=0;i<scene_cameras.size();i++){
			delete &scene_cameras[i].get();
		}scene_cameras.clear();
		string file_contents;
		getline(file,file_contents);
		vector<string> lvl_contents = split_string(file_contents,"|");
		for(int i=0;i<lvl_contents.size();i++){
			vector<string> saved_data = split_string(lvl_contents[i],";");
			int saved_type = stoi(saved_data[0]);
			if(saved_type == 0){
				Object* new_obj = new Object();
				new_obj->from_file(saved_data[1]);
				string_to_char(saved_data[2],new_obj->name[0]);
				new_obj->position = vector3_from_string(saved_data[3]);
				new_obj->scale = vector3_from_string(saved_data[4]);
				new_obj->rotation = vector3_from_string(saved_data[5]);
				scene_objects.push_back(*new_obj);
			}else if(saved_type == 1){
				Camera* new_cam = new Camera();
				string_to_char(saved_data[1],new_cam->name[0]);
				if((bool)stoi(saved_data[2]))
					current_camera = *new_cam;
				new_cam->position = vector3_from_string(saved_data[3]);
				new_cam->rotation.y = stof(saved_data[4]);
				new_cam->yaw = stof(saved_data[5]);
				scene_cameras.push_back(*new_cam);
			}
			saved_data.clear();
		}
		lvl_contents.clear();
	}
}
void save_scene(string path){

}
int width, height;
void run_engine(void (&pre_logic)(void (&)()),void(&logic)(void),int size_x,int size_y,string window_title,bool two_dimensional = false) {
	if (!glfwInit())
		return;
	window = glfwCreateWindow(size_x,size_y,window_title.c_str(),NULL,NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, setup_matrix);
	glfwGetFramebufferSize(window, &width, &height);
	glDrawBuffer(GL_FRONT);
	setup_matrix(window, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0, 0.0, -1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	double lastTime = glfwGetTime();
    int nbFrames = 0;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	scene_cameras.push_back(current_camera.get());

	while (!glfwWindowShouldClose(window))
	{
		deltaTime = getDeltaTime();
		HDC window_dc = wglGetCurrentDC();
		HWND window_handle = WindowFromDC(window_dc);
		glfwGetFramebufferSize(window, &width, &height);
		RECT window_rect;
		RECT display_rect;
		GetClientRect(window_handle, &display_rect);
		GetWindowRect(window_handle, &window_rect);
		POINT cursor_location;
		GetCursorPos(&cursor_location);
		changed_cursor_pos[0] = cursor_location.x;
		changed_cursor_pos[1] = cursor_location.y;
		default_cursor_pos[0] = floor(window_rect.left + (width / 2));
		default_cursor_pos[1] = floor(window_rect.top + (height / 2));
		if(cursor_lock&&(default_cursor_pos[0]!=changed_cursor_pos[0]||default_cursor_pos[1]!=changed_cursor_pos[1])) // Check cursor lock and if position changed
			SetCursorPos(default_cursor_pos[0],default_cursor_pos[1]);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// Drawing/Rendering objects
		for (int i = 0; i < scene_objects.size(); i++) {
			scene_objects[i].get().draw_mesh();
		}
		if(pre_logic_ran){
			logic();
		}else{
			pre_logic(logic);
		}
		updateTitleWithFPS(window,window_title,lastTime,nbFrames);
		ImGui::GetIO().IniFilename = NULL;
		glLoadIdentity();
		Vector3 up_vector = Vector3(0,1,0);
		Vector3 camera_rotation_offset = Vector3(.01,0,0);
		Vector3 camera_height_offset = Vector3(0,current_camera.get().yaw*0.01,0);
		Vector3 offset_camera = apply_vertex_rotation(camera_rotation_offset,current_camera.get().rotation)+current_camera.get().position+camera_height_offset;
		gluLookAt(
		current_camera.get().position.x
		,current_camera.get().position.y
		,current_camera.get().position.z
		,offset_camera.x
		,offset_camera.y
		,offset_camera.z
		,up_vector.x
		,up_vector.y
		,up_vector.z
		);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return;
}
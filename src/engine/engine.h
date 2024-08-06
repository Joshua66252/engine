#include "imgui.h" // Temporary until I make my own gui library
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
#define STB_IMAGE_IMPLEMENTATION
#include <engine/stb_image.h> // Temporary until I make my own image processor
using namespace std;
float fov = 80.0f;
float getDeltaTime() {
	static float lastFrame = 0.0f;
	float currentFrame = glfwGetTime();
	float deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	return deltaTime;
}
float deltaTime=getDeltaTime();
float sigmoid(float number) {
	return 1 / (1 + exp(-number));
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
struct Vector3{
	Vector3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	Vector3(float _x,float _y,float _z){
		x=_x;
		y=_y;
		z=_z;
	}
	bool operator==(const Vector3& _vector) const{
		return (this->x==_vector.x&&this->y==_vector.y&&this->z==_vector.z);
	}
	bool operator!=(const Vector3& _vector) const{
		return !(this->x==_vector.x&&this->y==_vector.y&&this->z==_vector.z);
	}
	Vector3 operator+(const Vector3& _vector) const{
		return Vector3(this->x+_vector.x,this->y+_vector.y,this->z+_vector.z);
	}
	Vector3 operator-(const Vector3& _vector) const{
		return Vector3(this->x-_vector.x,this->y-_vector.y,this->z-_vector.z);
	}
	Vector3 operator-(const float& num) const{
		return Vector3(this->x-num,this->y-num,this->z-num);
	}
	Vector3 operator*(const float& num) const{
		return Vector3(this->x*num,this->y*num,this->z*num);
	}
	Vector3 apply_rotation(Vector3 rotation){
		Vector3 vertex = Vector3(x,y,z);
		Vector3 _vertex = Vector3(x,y,z);
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
		vertex.y = _vertex.y;
		vertex.z = _vertex.z;
		_vertex.x = vertex.x * rotation_cos.y + vertex.z * rotation_sin.y;
		_vertex.z = vertex.z * rotation_cos.y - vertex.x * rotation_sin.y;
		vertex.x = _vertex.x;
		vertex.z = _vertex.z;
		_vertex.x = vertex.x * rotation_cos.z - vertex.y * rotation_sin.z;
		_vertex.y = vertex.y * rotation_cos.z + vertex.x * rotation_sin.z;
		vertex.x = _vertex.x;
		vertex.y = _vertex.y;
		return vertex;
	}
	double distance(Vector3 point){
		double distance = 0;
		double dx = point.x-x;
		double dy = point.y-y;
		double dz = point.z-z;
		dx = pow(dx,2);
		dy = pow(dy,2);
		dz = pow(dz,2);
		distance = dx+dy+dz;
		distance = sqrt(distance);
		return distance;
	}
	void from_str(string str){
		vector<string> vec = split_string(str,",");
		x=stof(vec[0]);
		y=stof(vec[1]);
		z=stof(vec[2]);
		vec.clear();
	}
	string to_str(){
		return to_string(x)+","+to_string(y)+","+to_string(z);
	}
	glm::vec3 to_glm(){
		return glm::vec3(x,y,z);
	}
	Vector3 cross_vectors(Vector3 vec){
		Vector3 result;
		result.x = y*vec.z-z*vec.y;
		result.y = z*vec.x-x*vec.z;
		result.z = x*vec.y-vec.y*x;
		return result;
	}
	float x;
	float y;
	float z;
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
class Camera{
	public:
		char name[2][128];
		Vector3 position = Vector3(0,0,0);
		Vector3 rotation = Vector3(0,1.57,0);;
		float yaw = 0;
	Camera() : name() {
		strcpy(name[0],"none");
	}
};
struct Mesh
{
	Mesh(){
		vertices = {
			Vector3(1, 1, -1),
			Vector3(1, -1, -1),
			Vector3(1, 1, 1),
			Vector3(1, -1, 1),
			Vector3(-1, 1, -1),
			Vector3(-1, -1, -1),
			Vector3(-1, 1, 1),
			Vector3(-1, -1, 1)
		};
		uv_vertices = {
			Vector3(1,1,0),
			Vector3(1,0,0),
			Vector3(0,0,0),
			Vector3(0,1,0)
		};
		faces = {
			{0, 4, 6, 2},
			{3, 2, 6, 7},
			{7, 6, 4, 5},
			{5, 1, 3, 7},
			{1, 0, 2, 3},
			{5, 4, 0, 1}
		};
		uv_faces={
			{0, 1, 2, 3},
			{0, 1, 2, 3},
			{0, 1, 2, 3},
			{0, 1, 2, 3},
			{0, 1, 2, 3},
			{0, 1, 2, 3}
		};
		colors = {
			Vector3(1, 1, 1)
		};
		vertex_colors = {
			0,0,0,0,0,0,0,0
		};
	}
	Mesh(vector<Vector3>& _vertices,vector<vector<unsigned int>>& _faces){
		vertices = _vertices;
		faces = _faces;
	}
	vector<Vector3> vertices;
	vector<Vector3> uv_vertices;
	vector<vector<unsigned int>> faces;
	vector<vector<unsigned int>> uv_faces;
	vector<Vector3> colors;
	vector<uint16_t> vertex_colors;
};
class Object{
	public:
		char name[2][128];
		Vector3 position;
		Vector3 rotation;
		Vector3 scale = Vector3(.125,.125,.125);
		Mesh mesh;
		GLuint texture_id = 0;
		bool wireframe = false;
		bool lighting = true;
		bool use_uv = true;
		int gl_face_culling = 1;
	Object() : name() {
		strcpy(name[0],"none");
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
			mesh.uv_faces.clear();
			mesh.uv_vertices.clear();
			mesh.vertices.clear();
			mesh.faces.clear();
			mesh.colors.clear();
			mesh.vertex_colors.clear();
			mesh.colors.push_back(Vector3(1,1,1));
			string current_line;
			vector<Vector3> vertices;
			vector<Vector3> uv_vertices;
			vector<vector<unsigned int>> uv_faces;
			vector<vector<unsigned int>> faces;
			vector<uint16_t> colors;
			while (getline(file, current_line)){
				vector<string> split;
				if (current_line.substr(0,2).compare("v ") == 0){
					split = split_string(current_line.substr(2)," ");
					Vector3 vertex(stof(split[0]),stof(split[1]),stof(split[2]));
					vertices.push_back(vertex);
					colors.push_back(0);
				}else if (current_line.substr(0,3).compare("vt ") == 0){
					split = split_string(current_line.substr(3)," ");
					Vector3 uv_vertex(1-stof(split[0]),1-stof(split[1]),0);
					uv_vertices.push_back(uv_vertex);
				}else if (current_line.substr(0,2).compare("f ") == 0){
					split = split_string(current_line.substr(2)," ");
					vector<string> _split;
					vector<unsigned int> face;
					vector<unsigned int> uv_face;
					for(int i=0;i<split.size();i++){
						_split = split_string(split[i],"/");
						face.push_back(stoi(_split[0])-1);
						uv_face.push_back(stoi(_split[1])-1);
					}
					faces.push_back(face);
					uv_faces.push_back(uv_face);
					face.clear();
					uv_face.clear();
					_split.clear();
				}
				split.clear();
			}
			mesh.uv_vertices = uv_vertices;
			uv_vertices.clear();
			mesh.uv_faces = uv_faces;
			uv_faces.clear();
			mesh.vertices = vertices;
			vertices.clear();
			mesh.faces = faces;
			faces.clear();
			mesh.vertex_colors = colors;
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
	void load_texture(string texture_path){
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		int width, height, nrChannels;
		unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;
			gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
		} else {
			std::cerr << "Failed to load texture: " << texture_id << std::endl;
		}
		stbi_image_free(data);
	}
	void draw_mesh(reference_wrapper<Camera>& current_cam,float lighting_severity){
		switch(gl_face_culling){
			case 0:
				glDisable(GL_CULL_FACE);
				break;
			case 1:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case 2:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			default:
				break;
		}
		glBindTexture(GL_TEXTURE_2D, texture_id);
		for(int i=0;i<mesh.faces.size();i++){
			bool actually_render = false;
			/*if(mesh_culling){
				for(int _i=0;_i<mesh.faces[i].size();_i++){
					Vector3 vertex = mesh.vertices[mesh.faces[i][_i]];
					vertex.x*=scale.x;
					vertex.y*=scale.y;
					vertex.z*=scale.z;
					vertex = vertex.apply_rotation(rotation);
					vertex = vertex+position;
					double distance_from_camera = current_camera.get().position.distance(vertex);
					float distance_factor = sigmoid(distance_from_camera)*(distance_from_camera*lighting_severity);
					if(distance_factor<=1){
						actually_render = true;
						break;
					}
				}
			}*/
			//if(actually_render||!mesh_culling){
				if(wireframe){
					glBegin(GL_LINE_LOOP);
				}else{
					glBegin(GL_POLYGON);
				}
				for(int _i=0;_i<mesh.faces[i].size();_i++){
					Vector3 vertex = mesh.vertices[mesh.faces[i][_i]];
					Vector3 uv_vertex;
					if(i<mesh.uv_faces.size()&&use_uv==true){
						if(mesh.uv_faces[i][_i]<mesh.uv_vertices.size()){
							uv_vertex = mesh.uv_vertices[mesh.uv_faces[i][_i]];
						}
					}else{
						if(_i==0){
							uv_vertex = Vector3(1, 1, 0);
						}else if(_i==1){
							uv_vertex = Vector3(1, 0, 0);
						}else if(_i==2){
							uv_vertex = Vector3(0, 0, 0);
						}else if(_i==3){
							uv_vertex = Vector3(0, 1, 0);
						}
					}
					vertex.x*=scale.x;
					vertex.y*=scale.y;
					vertex.z*=scale.z;
					vertex = vertex.apply_rotation(rotation);
					vertex = vertex+position;
					double distance_from_camera = current_cam.get().position.distance(vertex);
					float distance_factor = sigmoid(distance_from_camera)*(distance_from_camera*lighting_severity);
					Vector3 vertex_color = mesh.colors[mesh.vertex_colors[mesh.faces[i][_i]]];
					if(lighting){
						vertex_color = vertex_color-distance_factor;
						if(vertex_color.x<0)
							vertex_color.x=0;
						if(vertex_color.y<0)
							vertex_color.y=0;
						if(vertex_color.z<0)
							vertex_color.z=0;
					}
					glColor3f(vertex_color.x, vertex_color.y, vertex_color.z);
					glTexCoord2f(uv_vertex.x,uv_vertex.y);
					glVertex3f(vertex.x, vertex.y, vertex.z);
				}
				glEnd();
			//}
		}
	}
};
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
auto setup_matrix = [](GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, static_cast<float>(width) / static_cast<float>(height), 0.0001f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
};
struct Scene{
	Scene(){
		cursor_lock = false;
		hide_cursor = true;
		pre_logic_ran = false;
		lighting_severity = .7;
		object_culling_types[0] = "None";
		object_culling_types[1] = "Back face";
		object_culling_types[2] = "Front face";
	}
	GLFWwindow* window;
	int width, height;
	vector<reference_wrapper<Camera>> cameras;
	Camera* starting_camera = new Camera;
	reference_wrapper<Camera> current_camera = *starting_camera;
	vector<reference_wrapper<Object>> objects;
	uint_fast16_t default_cursor_pos[2];
	uint_fast16_t changed_cursor_pos[2];
	bool cursor_lock;
	bool hide_cursor;
	bool pre_logic_ran;
	float lighting_severity;
	const char* object_culling_types[3];
};
void built_in_movement(float move_speed,float turn_speed,Scene* scene){
	if(glfwGetKey(scene->window, GLFW_KEY_LEFT_SHIFT))
		scene->current_camera.get().position.y-=move_speed*deltaTime;
	if(glfwGetKey(scene->window, GLFW_KEY_SPACE))
		scene->current_camera.get().position.y+=move_speed*deltaTime;
	if(glfwGetKey(scene->window,GLFW_KEY_W)){
		Vector3 position_offset = Vector3(move_speed*deltaTime,0,0).apply_rotation(scene->current_camera.get().rotation);
		scene->current_camera.get().position = scene->current_camera.get().position+position_offset;
	}
	if(glfwGetKey(scene->window,GLFW_KEY_A)){
		Vector3 position_offset = Vector3(0,0,-move_speed*deltaTime).apply_rotation(scene->current_camera.get().rotation);
		scene->current_camera.get().position = scene->current_camera.get().position+position_offset;
	}
	if(glfwGetKey(scene->window,GLFW_KEY_S)){
		Vector3 position_offset = Vector3(-move_speed*deltaTime,0,0).apply_rotation(scene->current_camera.get().rotation);
		scene->current_camera.get().position = scene->current_camera.get().position+position_offset;
	}
	if(glfwGetKey(scene->window,GLFW_KEY_D)){
		Vector3 position_offset = Vector3(0,0,move_speed*deltaTime).apply_rotation(scene->current_camera.get().rotation);
		scene->current_camera.get().position = scene->current_camera.get().position+position_offset;
	}
	if(scene->cursor_lock&&(scene->default_cursor_pos[0]!=scene->changed_cursor_pos[0]||scene->default_cursor_pos[1]!=scene->changed_cursor_pos[1])){ // Check cursor lock and if position changed
		int_fast16_t mouse_movement[2] = {
			static_cast<int_fast16_t>(abs(scene->default_cursor_pos[0]-scene->changed_cursor_pos[0])), // Cast so compiler sees int to int16 is delibrate so that it won't show warnings
			static_cast<int_fast16_t>(abs(scene->default_cursor_pos[1]-scene->changed_cursor_pos[1]))
		};
		if(mouse_movement[0]>0){
			if(scene->default_cursor_pos[0]<scene->changed_cursor_pos[0])
				mouse_movement[0]=-mouse_movement[0];
			scene->current_camera.get().rotation.y+=turn_speed*mouse_movement[0];
		}
		if(mouse_movement[1]>0){
			if(scene->default_cursor_pos[1]<scene->changed_cursor_pos[1])
				mouse_movement[1]=-mouse_movement[1];
			scene->current_camera.get().yaw+=turn_speed*mouse_movement[1];
			if(scene->current_camera.get().yaw>2)
				scene->current_camera.get().yaw = 2;
			if(scene->current_camera.get().yaw<-2)
				scene->current_camera.get().yaw = -2;
		}
	}
}
void save_scene(string path, Scene* scene){
	ofstream scene_file;
	scene_file.open(path);
	scene_file.close();
}
void load_scene(string path,Scene* scene){
	if(path.size()<5){return;}
	string path_substring = path.substr(path.size()-5,5);
	transform(path_substring.begin(), path_substring.end(), path_substring.begin(), ::tolower);
	if (path_substring.compare("scene")==0){
		ifstream file(path);
		if (!file.is_open()){
			return;
		}
		for(int i=0;i<scene->objects.size();i++){
			delete &scene->objects[i].get();
		}
		scene->objects.clear();
		for(int i=0;i<scene->cameras.size();i++){
			delete &scene->cameras[i].get();
		}
		scene->cameras.clear();
		string file_contents;
		getline(file,file_contents);
		vector<string> lvl_contents = split_string(file_contents,"|");
		for(int i=0;i<lvl_contents.size();i++){
			vector<string> saved_data = split_string(lvl_contents[i],";");
			int saved_type = stoi(saved_data[0]);
			if(saved_type == 0){
				Object* new_obj = new Object();
				new_obj->from_file(saved_data[1]);
				strcpy(new_obj->name[0],saved_data[2].c_str());
				new_obj->position.from_str(saved_data[3]);
				new_obj->scale.from_str(saved_data[4]);
				new_obj->rotation.from_str(saved_data[5]);
				scene->objects.push_back(*new_obj);
			}else if(saved_type == 1){
				Camera* new_cam = new Camera();
				strcpy(new_cam->name[0],saved_data[1].c_str());
				if((bool)stoi(saved_data[2]))
					scene->current_camera = *new_cam;
				new_cam->position.from_str(saved_data[3]);
				new_cam->rotation.y = stof(saved_data[4]);
				new_cam->yaw = stof(saved_data[5]);
				scene->cameras.push_back(*new_cam);
			}
			saved_data.clear();
		}
		lvl_contents.clear();
	}
}
void run_engine(void (&pre_logic)(void (&)()),void(&logic)(void),int size_x,int size_y,string window_title,Scene* scene) {
	if (!glfwInit())
		return;
	scene->window = glfwCreateWindow(size_x,size_y,window_title.c_str(),NULL,NULL);
	if (!scene->window)
	{
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(scene->window);
	glfwSwapInterval(0);
	
	glfwSetFramebufferSizeCallback(scene->window, setup_matrix);
	glfwGetFramebufferSize(scene->window, &scene->width, &scene->height);
	glDrawBuffer(GL_FRONT);
	setup_matrix(scene->window, scene->width, scene->height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0, 0.0, -1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui_ImplGlfw_InitForOpenGL(scene->window, true);
	ImGui_ImplOpenGL3_Init();
	scene->cameras.push_back(scene->current_camera.get());
	while (!glfwWindowShouldClose(scene->window))
	{
		deltaTime = getDeltaTime();
		HDC window_dc = wglGetCurrentDC();
		HWND window_handle = WindowFromDC(window_dc);
		glfwGetFramebufferSize(scene->window, &scene->width, &scene->height);
		RECT window_rect;
		RECT display_rect;
		GetClientRect(window_handle, &display_rect);
		GetWindowRect(window_handle, &window_rect);
		POINT cursor_location;
		GetCursorPos(&cursor_location);
		scene->changed_cursor_pos[0] = cursor_location.x;
		scene->changed_cursor_pos[1] = cursor_location.y;
		scene->default_cursor_pos[0] = floor(window_rect.left + (scene->width / 2));
		scene->default_cursor_pos[1] = floor(window_rect.top + (scene->height / 2));
		if(scene->cursor_lock&&(scene->default_cursor_pos[0]!=scene->changed_cursor_pos[0]||scene->default_cursor_pos[1]!=scene->changed_cursor_pos[1])) // Check cursor lock and if position changed
			SetCursorPos(scene->default_cursor_pos[0],scene->default_cursor_pos[1]);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// Drawing/Rendering objects
		for (int i = 0; i < scene->objects.size(); i++) {
			scene->objects[i].get().draw_mesh(scene->current_camera,scene->lighting_severity);
		}
		if(scene->pre_logic_ran){
			logic();
		}else{
			pre_logic(logic);
		}
		updateTitleWithFPS(scene->window,window_title,lastTime,nbFrames);
		ImGui::GetIO().IniFilename = NULL;
		glLoadIdentity();
		Vector3 up_vector = Vector3(0,1,0);
		Vector3 camera_rotation_offset = Vector3(.01,0,0);
		Vector3 camera_height_offset = Vector3(0,scene->current_camera.get().yaw*0.01,0);
		Vector3 offset_camera = camera_rotation_offset.apply_rotation(scene->current_camera.get().rotation)+scene->current_camera.get().position+camera_height_offset;
		gluLookAt(
		scene->current_camera.get().position.x
		,scene->current_camera.get().position.y
		,scene->current_camera.get().position.z
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
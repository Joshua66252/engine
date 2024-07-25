#include <engine/engine.h>

void pre_logic(void (&logic)(void)){
	pre_logic_ran = true;
	load_scene("./default.scene");
	scene_objects[0].get().load_texture("container.jpg");
	logic();
}
float move_speed=1.3;
float turn_speed=.003;
bool pressing_escape = false;
bool pressing_debug = false;
bool debug_ui_open = true;
char temp[128];
char scene_name[128];
void vec3editor(string name,Vector3* vec3,int ID){
	ImGui::PushID(ID);
	ImGui::TextColored(ImVec4(1,1,0,1), string_to_char(name,temp));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::SliderFloat("##x",&vec3->x,-10.0f,10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::SliderFloat("##y",&vec3->y,-10.0f,10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::SliderFloat("##z",&vec3->z,-10.0f,10.0f);
	ImGui::PopID();
}
void obj_editor(Object* obj,int ID){
	char* object_name = obj->name[0];
	char* editor_name = obj->name[1];
	ImGui::PushID(ID);
	if (ImGui::TreeNode(string_to_char((string)object_name+"##"+to_string(ID),temp))){
		ImGui::SetNextItemWidth(300.0f);
		ImGui::InputText("##Text input",editor_name,128);
		if(ImGui::Button("Change name")){
			string_to_char(editor_name,object_name);
		}
		ImGui::SameLine();
		if(ImGui::Button("Load mesh")){
			obj->from_file(editor_name);
		}
		ImGui::SameLine();
		if(ImGui::Button("Delete object")){
			delete obj;
			scene_objects.erase(scene_objects.begin()+ID);
		}
		ImGui::TextColored(ImVec4(1,1,0,1),"Wireframe");
		ImGui::SameLine();
		ImGui::Checkbox("##Wireframe",&obj->wireframe);
		vec3editor("Scale",&obj->scale,0);
		vec3editor("Rotation",&obj->rotation,1);
		vec3editor("Position",&obj->position,2);
		ImGui::TextColored(ImVec4(1,1,0,1),"Culling");
		ImGui::ListBox("##cullingtype",&obj->gl_face_culling,object_culling_types,IM_ARRAYSIZE(object_culling_types));
		ImGui::TreePop();
	}
	ImGui::PopID();
}
void camera_editor(Camera* camera, int ID){
	char* camera_name = camera->name[0];
	char* editor_name = camera->name[1];
	ImGui::PushID(ID);
	if (ImGui::TreeNode(string_to_char((string)camera_name+"##"+to_string(ID),temp))){
		ImGui::SetNextItemWidth(300.0f);
		ImGui::InputText("##Text input",editor_name,128);
		if(ImGui::Button("Change name")){
			string_to_char(editor_name,camera_name);
		}
		ImGui::SameLine();
		if(ImGui::Button("Switch camera")){
			current_camera = *camera;
		}
		ImGui::SameLine();
		if(ImGui::Button("Delete Camera")){
			delete camera;
			scene_cameras.erase(scene_cameras.begin()+ID);
		}
		ImGui::TextColored(ImVec4(1,1,0,1),"Pitch and Yaw");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::SliderFloat("##Pitch",&camera->rotation.y,-10.0f,10.0f);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::SliderFloat("##Yaw",&camera->yaw,-10.0f,10.0f);
		vec3editor("Position",&camera->position,0);
		ImGui::TreePop();
	}
	ImGui::PopID();
}
void logic(){
	glfwPollEvents();
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(500, 800), ImGuiCond_Once);
	if(debug_ui_open){if (ImGui::Begin("Debug UI", &debug_ui_open)){
		ImGui::TextColored(ImVec4(1,1,0,1), "Rendering Stuff");
		ImGui::Checkbox("Lighting",&mesh_lighting);
		ImGui::SameLine();
		ImGui::Checkbox("Culling",&mesh_culling);
		ImGui::TextColored(ImVec4(1,1,0,1), "Movement Stuff");
		ImGui::SliderFloat("move_speed", &move_speed, 0.0f, 10.0f);
		ImGui::SliderFloat("turn_speed", &turn_speed, 0.0f, .01f);
		ImGui::SliderFloat("lighting_severity", &lighting_severity, 0.0f, 2.0f);
		if(ImGui::SliderFloat("fov",&fov,0.0f,120.0f)){
			setup_matrix(window, width, height);
		}
		ImGui::TextColored(ImVec4(1,1,0,1), "Misc Stuff");
		ImGui::SetNextItemWidth(300.0f);
		ImGui::InputText("##text input",scene_name,128);
		if(ImGui::Button("Load scene")){
			load_scene(scene_name);
		}
		ImGui::SameLine();
		if(ImGui::Button("Create object")){
			Object* new_obj = new Object();
			scene_objects.push_back(*new_obj);
		}
		ImGui::SameLine();
		if(ImGui::Button("Create camera")){
			Camera* new_cam = new Camera();
			scene_cameras.push_back(*new_cam);
		}
		if(ImGui::TreeNode("scene")){
			if (ImGui::TreeNode("cameras")){
				for(int i=0;i<scene_cameras.size();i++){
					camera_editor(&scene_cameras[i].get(),i);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("objects")){
				for(int i=0;i<scene_objects.size();i++){
					obj_editor(&scene_objects[i].get(),i);
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}else{
		ImGui::End();
	}}
	
	ImGuiIO& io = ImGui::GetIO();
	if(!io.WantCaptureKeyboard||!io.WantCaptureMouse){
		built_in_movement(move_speed,turn_speed);
		if(glfwGetKey(window,GLFW_KEY_MINUS)){
			move_speed-=.01;
			if(move_speed < 0)
				move_speed = 0;
		}
		if(glfwGetKey(window,GLFW_KEY_EQUAL)){
			move_speed+=.01;
		}
	}
	if(glfwGetKey(window,GLFW_KEY_F3)){
		if(!pressing_debug)
			debug_ui_open = !debug_ui_open;
		pressing_debug=true;
	}else{
		pressing_debug = false;
	}
	if(glfwGetKey(window,GLFW_KEY_ESCAPE)){
		if(!pressing_escape){
			cursor_lock=!cursor_lock;
			ShowCursor(!cursor_lock);
		}
		pressing_escape = true;
	}else{
		pressing_escape = false;
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glFlush();//glfwSwapBuffers(window);
}
int main(){
	run_engine(pre_logic,logic,1600,900,"J Engine Testing");
	return -1;
}
#include <GUI/GUI.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include <termios.h>

Renderer r = {0};
GLuint fbs, fb_tex;
XGL_t gl;
FILE* fp, *fout;
char channels;
unsigned *vb_c;

#define SQUARE "v -1.f -1.f 0.f 0.f\n"\
		"v 1.f -1.f 1.f 0.f\n"\
		"v -1.f 1.f 0.f 1.f\n"\
		"v 1.f 1.f 1.f 1.f\n"\
		"f 0 1 3\n"\
		"f 0 2 3"

char SHDR[] = "#shader vertex\n"
"#version 140\n"
"in vec2 inPos;\n"
"flat out int col_id;\n"
"uniform vec2 def_scl;\n"
"vec2 proj2(vec2 wp) {\n"
"        return vec2(def_scl.x * wp.x, def_scl.y * wp.y);\n"
"}\n"
"void main() {\n"
"       gl_Position = vec4(proj2(inPos), 0.f, 1.f);\n"
"	col_id = (inPos.y > 0)? 0:1;\n"
"}\n"
"#shader fragment\n"
"#version 140\n"
"flat in int col_id;\n"
"uniform vec4 col[10];\n"
"void main() {\n"
"	gl_FragColor = col[col_id];\n"
"}\n";

char SQR_SHDR[] = "#shader vertex\n"
"#version 140\n"
"in vec2 inPos;\n"
"in vec2 texCoord;\n"
"out vec2 uv_Out;\n"
"void main() {\n"
"        gl_Position = vec4(inPos, 0.f, 1.f);\n"
"        uv_Out = texCoord;\n"
"}\n"
"#shader fragment\n"
"#version 140\n"
"in vec2 uv_Out;\n"
"uniform sampler2D tex;\n"
"void main() {\n"
"	gl_FragColor = vec4(texture(tex, uv_Out));\n"
"}\n";

char FADE_SHDR[] = "#shader vertex\n"
"#version 140\n"
"in vec2 inPos;\n"
"in vec2 texCoord;\n"
"out vec2 uv_Out;\n"
"void main() {\n"
"        gl_Position = vec4(inPos, 0.f, 1.f);\n"
"        uv_Out = texCoord;\n"
"}\n"
"#shader fragment\n"
"#version 140\n"
"in vec2 uv_Out;\n"
"uniform sampler2D tex;\n"
"uniform float fade;\n"
"void main() {\n"
"	vec4 col = texture(tex, uv_Out)-vec4(fade);\n"
"	gl_FragColor = vec4(col.r, col.g, col.b, 1.f);\n"
"}\n";

FILE *openPort() {
	FILE *fp = NULL;

	char path[PATH_MAX] = "/dev/serial/by-id/";
	// First find the serial port that is open right now
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if(!d) {
		printf("No serial ports open\n");
		return NULL;
	}

	printf("Serial ports present\n");

	struct stat stat_buf = {0};

	while((dir = readdir(d)) != NULL) {
		if(!strncmp(dir->d_name, ".", 2) || !strncmp(dir->d_name, "..", 3))
			continue;
		strcpy(path+18, dir->d_name);
		if(lstat(path, &stat_buf) == -1) {
			fprintf(stderr, "could not get info of %s\n", path);
			switch(errno) {
				case EACCES:
					fprintf(stderr, "Permission denied\n");
					break;
				case ENOENT:
					fprintf(stderr, "No such file or dir\n");
					break;
				case ENOTDIR:
					fprintf(stderr, "Not a directory\n");
					break;
			}
			continue;
		}
		if(S_ISLNK(stat_buf.st_mode)) {
			char canon_path[PATH_MAX] = {0};
			if(realpath(path, canon_path) != NULL) {
				// This is 1 of the serial ports present
				printf("canon path = %s\n", canon_path);
				fp = fopen(canon_path, "r");
			}else {
				// This one isn't using symbolic links, weird
				fprintf(stderr, "Error getting realpath of %s\n", path);
			}
		}
	}

	return fp;
}

void background(float r, float g, float b) {
	GLCall(glClearColor(r, g, b, 1.f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Render() {
	//Draw the line just generated from serial port
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	RendererBind(r, 0, 0);
	VertexBufBind(r.models[0].vb);
	SendVBData(r.models[0].vb.mem+*vb_c-4*channels, 2*channels);
	Renderer_ALines(r, 0);
	VertexBufUnBind();

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, fb_tex));

	// Render everything to the screen
	RendererBind(r, 1, 1);
	Renderer_Fill(r, 1);

	glXSwapBuffers(gl.d, gl.w);
	
	// Apply the fade to the framebuffer
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	RendererBind(r, 1, 2);
	Renderer_Fill(r, 1);

	glBindTexture(GL_TEXTURE_2D, 0);
	RendererUnBind();
}

void my_exit() {
	Renderer_Delete(r);
	glxDestroyDisplay(gl);
	if(fp)fclose(fp);
	if(fout)fclose(fout);
}

void sync_input(unsigned char *in, char MAX_BYTES, const char* sync, FILE*fp) {	
	unsigned char *synk, bytes_read = 0;
	while(1) {
		fread(in, 1, 1, fp);
		while(*in != 'S') {// If SyNK packet is not aligned, skip this data
			fread(in, 1, 1, fp);
		}
		// Once SyNK is aligned, continue reading like normal
		bytes_read = fread(in+1, 1, MAX_BYTES-1, fp);
		// If data is still not aligned, align it and continue reading
		synk = memchr(in, 'S', bytes_read);
		if(synk == NULL) {
			continue;// Restart the loop again
		}
		break;
	}
	// If SyNK packet is found, then align it and read the remaining data
	unsigned char diff = synk - in;
	if(diff == 0) {
		// Data is aligned, so we return
		return;
	}
	// Align data
	int off = MAX_BYTES-diff;
	memmove(in, synk, off);
	bytes_read = 0;
	while(bytes_read != diff) {
		bytes_read = fread(in+off, 1, diff, fp);
		off += bytes_read;
		diff -= bytes_read;
	}
	return;
}

int main(int argc, char** argv) {
	atexit(my_exit);

	int width = 1000, height = 400;
	channels = 1;

	if(argc > 1) {
		switch(argv[1][0]) {
			case '-':
				if(argv[1][1] == 'h')printf("usage: %s [DEFAULT=1] channels, [OPTIONAL] width, [OPTIONAL] height\n", argv[0]);
				return 1;
		}
		if(argc == 2) {
			channels = (char)atoi(argv[1]);
			printf("channels = %d\n", channels);
			if(argc > 2) {
				width = atoi(argv[2]);
				height = atoi(argv[3]);
			}
		}
	}

	// Initialise vb_c
	vb_c = malloc(channels * sizeof(*vb_c));

	XGLOpenWindow(&gl, width, height, "Lab_Renderer",  ExposureMask | KeyPressMask);

	r = init_Renderer(2, 3);
	Renderer_AddDModel(r, 0, 4*width, 0, "2f");
	Renderer_AddModel(r, SQUARE, 1, "2f 2f");
	r.shaders[0] = init_ShaderStr(SHDR);
	r.shaders[1] = init_ShaderStr(SQR_SHDR);
	r.shaders[2] = init_ShaderStr(FADE_SHDR);

	GLCall(glGenFramebuffers(1, &fbs));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	GLCall(glCheckFramebufferStatus(GL_FRAMEBUFFER));

	GLCall(glGenTextures(1, &fb_tex));
	GLCall(glBindTexture(GL_TEXTURE_2D, fb_tex));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0));

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Error with GL framebuffer\n");
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GLCall(glDeleteFramebuffers(1, &fbs));
		return 2;
	}

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	background(0, 0, 0);

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	float aspect[2] = {1.f/width, 1.f/height};
	// Set up uniforms
	ShaderBind(r.shaders[0]);
	Shader_SetUniform2fv(r.shaders[0], "def_scl", aspect, 1);
	ShaderBind(r.shaders[1]);
	Shader_SetUniform1i(r.shaders[1], "tex", 0);
	ShaderBind(r.shaders[2]);
	Shader_SetUniform1i(r.shaders[2], "tex", 0);
	Shader_SetUniform1f(r.shaders[2], "fade", 0/*0.002*/);

	ShaderUnBind();

	// Start random for testing
	srandom(time(NULL));

	fp = openPort();
	fout = fopen("output.csv", "w");
	if(!fp) {
		printf("Couldn't open any serial ports\n");
		return 3;
	}

	if(!fout) {
		printf("Couldn't open log file\n");
		return 4;
	}

	// Setup The Baud Rate of the port
	int fp_fd = fileno(fp);
	struct termios tty;
	memset(&tty, 0, sizeof(tty));
	if(tcgetattr(fp_fd, &tty) != 0) {
		printf("Error getting terminal attributes of Port\n");
		return 5;
	}

	cfsetispeed(&tty, B9600);
	tty.c_cflag |= (CLOCAL | CREAD);// Enable receiver
	tty.c_cflag &= ~PARENB;// No parity bit
	tty.c_cflag &= ~CSTOPB;// 1 Stop bit
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;// 8 data bits
	tcsetattr(fp_fd, TCSANOW, &tty);

	// We're gonna read till \n char then display the output onto the screen
	// So before connecting to the board we gotta setup the screen first

	int xPos = 0;
	char rendr_buf = 0;
	{
		ShaderBind(r.shaders[0]);
		float col[8] = {1.f, 0, 0, 1.f, 0.f, 0.f, 1.f, 1.f};
		Shader_SetUniform4fv(r.shaders[0], "col", col, 2);
	}

	for(int i = 0; i < channels; i++)
		vb_c[i] = 2*width*i;
	for(int i = 0; i < channels; i++) {
		r.models[0].vb.mem[vb_c[i]++] = -width;
		r.models[0].vb.mem[vb_c[i]++] = 0;
	}

	// Sync up with the arduino b4 starting
	const char sync[] = "SyNK";
	unsigned char input[20] = {0};
	char MAX_BYTES = 2*channels+4;

	while(1) {
		// Read in serial data from the board and plot the point on the display
		// Also save the data to a csv file
		// Probably just use a separate thread to save the data, cause of how slow it is to write to files?? or maybe not, or maybe yes
		// We will need to keep 2 buffers 1 to save to file and the other for the screen's current display
		sync_input(input, MAX_BYTES, sync, fp);
		//if(!bytes_read) bytes_read = fread(input, 1, MAX_BYTES, fp);

		short heights[100] = {0};
		
		//printf("input = {%.8s}%d, %d\n", input, MAX_BYTES, bytes_read);
		memcpy(heights, input+4, 2*channels);
		//printf("heights = %hi, %hi\n", heights[0], heights[1]);
		//for(int i = 0; i < channels; i++) heights[i] -= i*512;
		//height0_new = random()%1024;
		//height1_new = random()%1024-1024;

		for(int i = 0; i < channels; i++) {
			r.models[0].vb.mem[vb_c[i]++] = xPos*2-width;
			r.models[0].vb.mem[vb_c[i]++] = heights[i] * height / 1024;
		}	

		for(int j = 1; j < channels; j++) {
			for(int i = -4; i < 0; i++) {
				r.models[0].vb.mem[vb_c[0]++] = r.models[0].vb.mem[vb_c[j]+i];
			}
		}
		// Now we have to update the shader

		Render(rendr_buf);
		vb_c[0] -= 4*(channels-1);
		if(xPos == width) {
			xPos = 0;
			//fwrite(r.models[0].vb.mem, 4, width*4, fout);
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
			background(0, 0, 0);
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			for(int i = 0; i < channels; i++)
				vb_c[i] = 2*width*i;
			for(int i = 0; i < channels; i++) {
				r.models[0].vb.mem[vb_c[i]++] = -width;
				r.models[0].vb.mem[vb_c[i]++] = 0;
			}
		}else {
			xPos++;
		}
	}

	return 0;
}

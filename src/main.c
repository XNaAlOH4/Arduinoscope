#include <GUI/GUI.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <termios.h>

#include <poll.h>

Renderer r = {0};
GLuint fbs, fb_tex;
XGL_t gl;

int fp_fd;
FILE*fout;
char channels, option;
short *heights;
unsigned *vb_c;

#define EFFECTS 0

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
"	col_id = 0;\n//(inPos.y > 0)? 0:1;\n"
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

int openSerialPort() {
	int fd = -1;

	char path[PATH_MAX] = "/dev/serial/by-id/";
	// First find the serial port that is open right now
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if(!d) {
		printf("No serial ports open\n");
		return fd;
	}

	//printf("Serial ports present\n");

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
				fd = open(canon_path, O_RDONLY, O_NONBLOCK);

				// Setup The Baud Rate of the port
				struct termios tty;
				memset(&tty, 0, sizeof(tty));
				if(tcgetattr(fd, &tty) != 0) {
					printf("Error getting terminal attributes of Port\n");
					return -1;
				}

				cfsetispeed(&tty, B9600);// Baud rate of 9600
				tty.c_cflag |= CS8;// 8 data bits
				tty.c_cflag &= ~PARENB;// No parity bit
				tty.c_cflag &= ~CRTSCTS;
				tty.c_cflag &= ~CSTOPB;// 1 Stop bit
				
				tty.c_lflag = 0;

				tty.c_cc[VMIN] = 1;
				tty.c_cc[VTIME] = 10;

				tcsetattr(fd, TCSANOW, &tty);
				break;
			}else {
				// This one isn't using symbolic links, weird
				fprintf(stderr, "Error getting realpath of %s\n", path);
			}
		}
	}

	return fd;
}

void background(float r, float g, float b) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbs);
	GLCall(glClearColor(r, g, b, 1.f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render() {
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	RendererBind(r, 0, 0);
	VertexBufBind(r.models[0].vb);
	SendVBData(r.models[0].vb.mem+*vb_c-4*channels, 2*channels);

	//Draw the line just generated from serial port
	Renderer_ALines(r, 0);
	VertexBufUnBind();

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, fb_tex));

	// Render everything to the screen
	RendererBind(r, 1, 1);
	Renderer_Fill(r, 1);

	glXSwapBuffers(gl.d, gl.w);
#if EFFECTS
	// Apply the fade to the framebuffer
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	RendererBind(r, 1, 2);
	Renderer_Fill(r, 1);

	//glBindTexture(GL_TEXTURE_2D, 0);
#endif

	RendererUnBind();
}

void Render_Scatter(int packets) {
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbs));
	RendererBind(r, 0, 0);
	VertexBufBind(r.models[0].vb);
	SendVBData(r.models[0].vb.mem+*vb_c-2*packets, 2*packets);

	//Draw the dots just generated from serial port
	Renderer_ADot(r, 0);
	VertexBufUnBind();

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, fb_tex));

	// Render everything to the screen
	RendererBind(r, 1, 1);
	Renderer_Fill(r, 1);

	glXSwapBuffers(gl.d, gl.w);
}

void my_exit() {
	Renderer_Delete(r);
	glxDestroyDisplay(gl);
	if(fp_fd != -1)close(fp_fd);
	if(fout)fclose(fout);
}

void sync_input(unsigned char *in, char MAX_BYTES, int fd) {	

	unsigned char *synk, bytes_read = 0;
	while(1) {
		read(fd, in, 1);
		while(*in != 'S') {// If SyNK packet is not aligned, skip this data
			//printf("Out of sync\n");
			read(fd, in, 1);
		}
		// Once SyNK is aligned, continue reading like normal
		bytes_read = read(fd, in+1, MAX_BYTES-1);
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
		bytes_read = read(fd, in+off, diff);
		off += bytes_read;
		diff -= bytes_read;
	}
	return;
}

void KeyDown(unsigned keycode)
{
	switch(keycode) {
		case xkey_q:
			exit(0);
		case xkey_p:
			for(int i = 0; i < channels; i++) printf("%hi(%f), ", heights[i], heights[i]/1023.f*5.f);
			printf("\n");
			break;
		case xkey_o:
			option ^= 1;
			break;
	}
}

int main(int argc, char** argv) {
	// SETUP CODE //
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
	vb_c = malloc(2 * channels * sizeof(*vb_c));

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

	fp_fd = openSerialPort();
	fout = fopen("output.csv", "w");
	if(fp_fd == -1) {
		printf("Couldn't open any serial ports\n");
		return 3;
	}

	if(!fout) {
		printf("Couldn't open log file\n");
		return 4;
	}

	// We're gonna read till \n char then display the output onto the screen
	// So before connecting to the board we gotta setup the screen first

	int xPos = -width;
	{
		ShaderBind(r.shaders[0]);
		float col[8] = {1.f, 0, 0, 1.f, 0.f, 0.f, 1.f, 1.f};
		Shader_SetUniform4fv(r.shaders[0], "col", col, 2);
	}

	for(int i = 0; i < channels; i++) {
		vb_c[i] = 2*width*i;
		r.models[0].vb.mem[vb_c[i]++] = -width;
		r.models[0].vb.mem[vb_c[i]++] = 0;
	}

	struct pollfd fds[] = {
		{
			ConnectionNumber(gl.d),
			POLLIN,
			0,
		},
		{
			fp_fd,
			POLLIN,
			0,
		},
	};

	// END OF SETUP CODE //

	char packets = 2;// I think the arduino might not be able to handle sending out 4 packets so fast or maybe data was just corrupted along the way, who knows
	unsigned char input[(4+2*2)*5] = {0};
	char MAX_BYTES = (2*channels+4)*packets;
	heights = (short*)(input+4);

	short *mem = malloc(sizeof(short)*2*width*2);
	memset(mem, 0, sizeof(short)*2*2*width);

	option = 1;

	// Sync up with the arduino b4 starting
	// Sync the computer and arduino but sadly have to discard the first value
	sync_input(input, MAX_BYTES/packets, fp_fd);

#define MY_POLL_TIMEOUT -1

	while(1) {
		if(poll(fds, 2, MY_POLL_TIMEOUT) < 1) continue;

		if(fds[0].revents) {
			if(fds[0].revents & POLLIN) {
				int pending_events = XPending(gl.d);
				for(int i = 0; i < pending_events; i++) {
					XNextEvent(gl.d, &gl.ev);
					switch(gl.ev.type) {
						case ClientMessage:
							if(gl.ev.xclient.data.l[0] == gl.delwindow)
								glxDestroyDisplay(gl);
							break;
						case KeyPress:
							KeyDown(gl.ev.xkey.keycode);
							break;
					}
				}
			}else {
				printf("X event polling error: %x\n", fds[0].revents);
			}
		}

		if(fds[1].revents) {
		if(fds[1].revents & POLLIN) {
			// first read in data, if not aligned then align it
			memset(input, 0, MAX_BYTES);
			// Clear data, because this might be the one causing anomalous data when not enough bytes are read
			sync_input(input, MAX_BYTES, fp_fd);
			char off = MAX_BYTES;

			char anomalous;

			//Test data for bounds
			char skip_packs = 0;
			for(int j = 0; j < packets; j++) {
				anomalous = 0;
				for(int i = 0, k = j*4; i < channels; i++, k++) {
					if((heights[k] < 0) || (heights[k] > 1023)) {
						//printf("Boundskip %hd\n", heights[i]);
						anomalous = 1;
						break;// Skip weird data
					}

					int vbc = (xPos == 0 && j == 0)? 2*(k+1)*width+3:vb_c[k]-1;
					short previous = r.models[0].vb.mem[vbc] / height * 1023;
					if(heights[k] > previous+600) {
						//printf("Upskip: p,n(%hd, %hd)\n", previous, heights[i]);
						anomalous = 1;
						break;
					}
					if(heights[k] < previous-600) {
						//printf("Downskip: p,n(%hd, %hd)\n", previous, heights[i]);
						anomalous = 1;
						break;
					}
				}
				if(anomalous) skip_packs |= (1<<j);
			}

			if(skip_packs&((1<<packets)-1)) continue;// If all packets are anomalous, skip it

			if(option) {
				int added = 0;
				float Vd, I;
				// Render Diode I/V graph
				for(int i = 0; i < packets; i++) {
					if((skip_packs>>i) & 1) continue;
					Vd = (heights[i*4] - heights[i*4+1])/1023.f*height;
					I = heights[i*4+1]*height/218.f;
					//printf("Rendering: %f, %f\n", Vd/height*5, I/height*5.f/1023.f);
					if(Vd < 0 || I < 0) {
						//printf("Error: %hi, %hi, %.8s\n", heights[i*4], heights[i*4+1], input+8*i);
						continue;
					}
					added++;
					mem[vb_c[i]] = heights[i*4]-heights[i*4+1];
					r.models[0].vb.mem[vb_c[i]++] = Vd;
					mem[vb_c[i]] = heights[i*4+1];
					r.models[0].vb.mem[vb_c[i]++] = I;
				}

				if(added == 0) continue;

				Render_Scatter(added);
				xPos += packets;
				if(xPos >= width) {
					printf("Done\n");
					xPos = -width;
					fwrite(mem, 2, width*2*2, fout);
					background(0, 0, 0);
					for(int i = 0; i < channels; i++) vb_c[i] = 2*width*i+2;
				}
				continue;
			}

			if(skip_packs&1) continue;
			for(int i = 0; i < channels; i++) {
				r.models[0].vb.mem[vb_c[i]++] = xPos;
				r.models[0].vb.mem[vb_c[i]++] = heights[i] * height / 1023;
			}

			for(int j = 1; j < channels; j++) {
				for(int i = -4; i < 0; i++) {
					r.models[0].vb.mem[vb_c[0]++] = r.models[0].vb.mem[vb_c[j]+i];
				}
			}

			Render();
			vb_c[0] -= 4*(channels-1);

			xPos += 2;
			if(xPos >= width) {
				xPos = -width;
				//fwrite(r.models[0].vb.mem, 4, width*2*channels, fout);
				background(0, 0, 0);
				for(int i = 0; i < channels; i++) vb_c[i] = 2*width*i+2;
			}

			// Test for over-read due to desync
			/*if(diff < 2) {
				off = 0;
			}*/
		}else {
			printf("file event error: %x\n", fds[1].revents);
		}
		}
	}

	return 0;
}

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h> //Future multithreading
#include <time.h>

const int NUM_THREADS = 4; //Number of threads to dial the work to.
const long int COLOR_SHIFT_RED = 10; //Amount to shift the red up.
const long int COLOR_SHIFT_BLUE = 10; //Amount to shift the blue up.
const long int COLOR_SHIFT_GREEN = 10; //Amount to shift the green up.
const int SCANLINE = 2; //Seperation between the scanlines.
const long int SCANLINE_DIM = -50; //Amount to dim scanlines.

struct fb_var_screeninfo var_info; //Contains the variables of the screen.
struct fb_fix_screeninfo fix_info; //Contains the fixed info of the screen.

struct Screen
{
	long int screen_size; //Contains the screen's size in bytes.
	char* frame_buffer; //Pointer to the framebuffer in memory.
	int frame_buffer_fd; //File descriptor for the frame buffer.
	long int location; //Current editing location.
	short current_color; //Tracks the current color
};

/*  Prints an error to the screen
	@param char* - the error message to print.
	@param int - the error code to exit by.
*/
void error(char* error_message, int error_code)
{
	perror(error_message);
	exit(error_code);
}

/*  Opens the frame buffer for writing.
	@param my_screen - screen variables.
*/
void open_frame_buffer(struct Screen* my_screen)
{
	my_screen->frame_buffer_fd = open("/dev/fb0", O_RDWR); //Open next frame to be rendered.
	if(my_screen->frame_buffer_fd == -1)
	{
		error("Unable to open the framebuffer device!", 1);
	}
}

/*  Fills Screen with the screen's fixed info.
	@param my_screen - screen variables
*/
void get_fixed_info(struct Screen* my_screen)
{
	if(ioctl(my_screen->frame_buffer_fd, FBIOGET_FSCREENINFO, &fix_info) == -1)
	{
		error("Unable to read fixed screen information!", 2);
	}
}

/*  Gets the screen's variable info.
	@param my_screen - screen variables.
*/
void get_variable_info(struct Screen* my_screen)
{
	if(ioctl(my_screen->frame_buffer_fd, FBIOGET_VSCREENINFO, &var_info) == -1)
	{
		error("Unable to read variable screen information!", 3);
	}
}

/*  Calculates the screens filesize and maps the frame buffer in memory.
	@param my_screen - screen variables.
*/
void map_screen(struct Screen* my_screen)
{
	//Calculate screen size in bytes.
	my_screen->screen_size = var_info.xres * var_info.yres * var_info.bits_per_pixel / 8;
	my_screen->frame_buffer = (char*)mmap(0, my_screen->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED,
								my_screen->frame_buffer_fd, 0);
	if(my_screen->frame_buffer == (char*)-1)
	{
		error("Unable to map the framebuffer device to memory!", 4);
	}
	my_screen->location = 0;
	my_screen->current_color = 0;
}

/*  Fills out the members of a my_screen struct.
	@return my_screen - screen struct initialized.
*/
struct Screen* init_screen()
{
	struct Screen* my_screen = malloc(sizeof(my_screen));
	open_frame_buffer(my_screen);
	get_fixed_info(my_screen);
	get_variable_info(my_screen);
	map_screen(my_screen);
	return my_screen;
}

/*  Closes the screen file descriptor and releases objects from memory.
	@param my_screen - screen information to close.
*/
void close_screen(struct Screen* my_screen)
{
	close(my_screen->frame_buffer_fd); // close fd
	munmap(my_screen->frame_buffer, my_screen->location); //unmap memory.
	free(my_screen->frame_buffer); //Free pointer
	free(my_screen); //Free struct
}

/*  Updates the screen with the desired effect.
	@param my_screen - screen information to update.
*/
void update_screen_32bpp(struct Screen* my_screen)
{
	//General code, will probably need to change variables.
	for(int y = 0; y < var_info.yres; y++)
	{
		for(int x = 0; x < var_info.xres; x++)
		{
			//Current location where editing.
			my_screen->location = (x + var_info.xoffset) * (var_info.bits_per_pixel / 8) +
									(y + var_info.yoffset) * fix_info.line_length;
			//The biggest of endians.
			//Todo: handle pixel wraparound using mod.
			if(my_screen->current_color == 2)
			{
				*(my_screen->frame_buffer + my_screen->location) += COLOR_SHIFT_BLUE; //Blue.
				my_screen->current_color = 0;
			}
			else if(my_screen->current_color == 1)
			{
				*(my_screen->frame_buffer + my_screen->location + 1) += COLOR_SHIFT_GREEN; //Green.
				my_screen->current_color++;
			}
			else if(my_screen->current_color == 0)
			{			
				*(my_screen->frame_buffer + my_screen->location + 2) += COLOR_SHIFT_RED; //Red.
				my_screen->current_color++;
			}
			if(x % SCANLINE == 0)
			{
				*(my_screen->frame_buffer + my_screen->location + 3) += SCANLINE_DIM; //Alpha channel.
			}
		}
	}
}

//TODO: WRITE THIS FUNCTION.
void update_screen_24bpp(struct Screen* my_screen)
{
	//General code, will probably need to change variables.
	for(int y = 0; y < var_info.yres; y++)
	{
		for(int x = 0; x < var_info.xres; x++)
		{
			error("24 bit screens are currently unsupported.", 5);
		}
	}
}

void update_screen_16bpp(struct Screen* my_screen)
{
	//General code, will probably need to change variables.
	for(int y = 0; y < var_info.yres; y++)
	{
		for(int x = 0; x < var_info.xres; x++)
		{
			//TODO
			error("16 bit screens are currently unsupported.", 6);
		}
	}
}

/*
	TODO LIST:
	1. Get screen refresh rate to stay in sync.
	2. Write thread pool.
	3. Write shader effect.
	4. Write to buffer.
	5. Add user options.
*/
int main(int argc, char* argv[])
{
	struct Screen* my_screen = init_screen();
	printf("INFO: xres: %d, yres: %d\n", var_info.xres, var_info.yres);
	update_screen_32bit(my_screen);
	close_screen(my_screen);
	return 0;
}

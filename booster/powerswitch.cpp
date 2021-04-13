extern "C" {
void set_screen_on(int);
}

int main(int, char**)
{
	// Used to trigger screen-on behavior when starting the LXC container
	set_screen_on(0);
	return 0;
}

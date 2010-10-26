/* fakelcd.c */

#include <glib.h>
#include <gtk/gtk.h>

/* Backing pixmap for drawing area */
static GdkPixmap *pixmap = NULL;
static GtkWidget *canvas = NULL;
static GdkGC *gc = NULL;
static GdkColor color;
static unsigned int lcd_address;

#define LCD_WIDTH	240
#define LCD_HEIGHT	320

static void lcd_set_address(unsigned int address)
{
	lcd_address = address;
}

static void _lcd_write(unsigned int address, unsigned short data)
{
	gint x;
	gint y;
	gint width;
	gint height;
	unsigned int aligned_address = address & ~0x1;

	gdk_window_get_geometry(canvas->window, NULL, NULL, &width, &height, NULL);

	if (aligned_address < ((width * 2) * height))
	{
		color.red = ((data & 0xf800) >> 11) * (16 * 1024 / 5);
		color.green = ((data & 0x7e0) >> 5) * (16 * 1024 / 6);
		color.blue = (data & 0x1f) * (16 * 1024 / 5);

		x = (aligned_address / 2) % width;
		y = (aligned_address / 2) / width;

		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_draw_point(pixmap, gc, x, y);
		gtk_widget_queue_draw_area(canvas, x, y, 1, 1);
	}
	else
	{
		g_print("Invalid address: 0x%08x\n", address);
	}
}

static inline void lcd_write(unsigned short data)
{
	_lcd_write(lcd_address, data);
}

static void lcd_draw_point(unsigned int x, unsigned int y, unsigned short data)
{
	gint width;
	gint height;
	gdk_window_get_geometry(canvas->window, NULL, NULL, &width, &height, NULL);

	if (x < width && y < height)
	{
		lcd_set_address(x * 2 + (y * width * 2));
		lcd_write(data);
	}
	else
	{
		g_print("Invalid coordinate: x=%d, y=%d\n", x, y);
	}
}

/* Create a new backing pixmap of the appropriate size */
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	if (pixmap)
		gdk_drawable_unref(pixmap);

	pixmap = gdk_pixmap_new(widget->window,
			widget->allocation.width,
			widget->allocation.height,
			-1);

	gc = gdk_gc_new(GDK_DRAWABLE(pixmap));

	gdk_draw_rectangle(pixmap,
			widget->style->black_gc,
			TRUE,
			0, 0,
			widget->allocation.width,
			widget->allocation.height);

	return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gint expose_event(GtkWidget *widget, GdkEventExpose *event)
{
	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}

static void print_button_press(GdkEventButton *event)
{
	g_print("Click: %d, %d\n", (int)event->x, (int)event->y);
}

static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	print_button_press(event);

	if (pixmap)
	{
		unsigned short data = 0;
		g_print("Button: %d\n", event->button);

		switch (event->button)
		{
			case 1:
				data = 0xf800;
				break;
			case 2:
				data = 0x1f;
				break;
			case 3:
				data = 0x7e0;
				break;
			default:
				break;
		}

		lcd_draw_point(event->x, event->y, data);
	}

	return TRUE;
}

int main(int argc, char *argv[])
{
	/* Declare the GTK Widgets used in the program */
	GtkWidget *window;

	/*  Initialize GTK */
	gtk_init(&argc, &argv);

	/* Create the new window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "dispsim");
	//gtk_container_set_border_width(GTK_CONTAINER(window), 1);

	gtk_signal_connect(GTK_OBJECT(window), "destroy",
			GTK_SIGNAL_FUNC(gtk_exit), NULL);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request(canvas, LCD_WIDTH, LCD_HEIGHT);
	gtk_container_add(GTK_CONTAINER(window), canvas);
	gtk_widget_show(canvas);

	/* Signals used to handle backing pixmap */
	gtk_signal_connect(GTK_OBJECT(canvas), "expose_event",
			(GtkSignalFunc)expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(canvas),"configure_event",
			(GtkSignalFunc)configure_event, NULL);

	/* Event signals */
	gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event",
			(GtkSignalFunc)button_press_event, NULL);

	gtk_widget_set_events (canvas, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK);

	/*
	 * The following call enables tracking and processing of extension
	 * events for the drawing area
	 */
	gtk_widget_set_extension_events(canvas, GDK_EXTENSION_EVENTS_CURSOR);

	/* Display the window */
	gtk_widget_show(window);

	/* Start the GTK event loop */
	gtk_main();

	gdk_gc_unref(gc);

	/* Return 0 if exit is successful */
	return 0;
}

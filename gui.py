import tkinter as tk
from config import UPDATE_INTERVAL_MS, FONT_FAMILY, FONT_SIZE, COLOR_DEFAULT, COLOR_HIGHLIGHT, TRANSPARENT_COLOR, GAP_X, GAP_Y
from utils import get_system_info_text

def create_gui():
    """Set up and run the Tkinter GUI."""
    root = tk.Tk()
    root.title("Desk Info")
    root.config(bg=TRANSPARENT_COLOR)
    root.overrideredirect(True)

    try:
        root.wm_attributes("-transparentcolor", TRANSPARENT_COLOR)
    except Exception:
        pass

    font_config = (FONT_FAMILY, FONT_SIZE)
    info_label = tk.Text(
        root,
        width=50,
        font=font_config,
        fg=COLOR_DEFAULT,
        bg=TRANSPARENT_COLOR,
        padx=5,
        pady=5,
        highlightthickness=0,
        borderwidth=0,
        cursor="arrow"
    )
    info_label.pack()

    info_label.tag_configure('nodename_style',
                            foreground=COLOR_HIGHLIGHT,
                            font=(FONT_FAMILY, 16, 'bold'),
                            justify='center')

    info_label.tag_configure('separator_style',
                            foreground=COLOR_HIGHLIGHT,
                            font=(FONT_FAMILY, FONT_SIZE),
                            justify='center')

    def block_event(event):
        return "break"

    blocked_events = ("<Button-1>", "<B1-Motion>", "<Control-c>", "<Control-C>",
                      "<Control-x>", "<Control-v>", "<Key>")
    for ev in blocked_events:
        info_label.bind(ev, block_event)

    def update_info_label():
        try:
            info_text = get_system_info_text()
            lines = info_text.splitlines()

            if len(lines) >= 2:
                nodename = lines[0]
                separator_line = lines[1]
                rest_of_text = "\n".join(lines[2:]) if len(lines) > 2 else ""
            elif len(lines) == 1:
                nodename = lines[0]
                separator_line = "-" * 50
                rest_of_text = ""
            else:
                nodename = "N/A"
                separator_line = "-" * 50
                rest_of_text = ""

            info_label.delete('1.0', tk.END)
            info_label.insert('1.0', f"{nodename}\n", 'nodename_style')
            info_label.insert(tk.END, f"{separator_line}\n", 'separator_style')
            if rest_of_text:
                info_label.insert(tk.END, rest_of_text)
        except Exception:
            info_label.delete('1.0', tk.END)
            info_label.insert('1.0', "Erro ao coletar informações")

        root.after(UPDATE_INTERVAL_MS, update_info_label)

    root.update_idletasks()
    screen_width = root.winfo_screenwidth()
    window_width = root.winfo_width()
    pos_x = screen_width - window_width - GAP_X
    pos_y = GAP_Y
    root.geometry(f"+{pos_x}+{pos_y}")

    update_info_label()
    root.mainloop()
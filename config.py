# --------------------------------------------------------------
# Configuration constants
# --------------------------------------------------------------
UPDATE_INTERVAL_MS = 2000
FONT_FAMILY = "Consolas"
FONT_SIZE = 11
GAP_X = 30
GAP_Y = 30
COLOR_DEFAULT = "white"
COLOR_HIGHLIGHT = "dodgerblue"
TRANSPARENT_COLOR = "black"
MUTEX_NAME = "DeskInfo_SingleInstance_Mutex"
SEPARATOR = "-" * 50

# --------------------------------------------------------------
# Default configuration file content (template)
# --------------------------------------------------------------
DEFAULT_CFG = r"""Machine Domain:   {machine_domain}
IP Address:       {ip_address}
{SEPARATOR}
User Name:        {user_name}
Logon Domain:     {logon_domain}
{SEPARATOR}
OS Version:       {os_version}
System Type:      {system_type}
{SEPARATOR}
CPU Usage:        {cpu_usage}
Memory:           {mem_used_gb} / {mem_total_gb}
Disk Usage (C:\): {disk_c_usage}
{disk_info}
"""

# --------------------------------------------------------------
# Default README file content
# --------------------------------------------------------------
DEFAULT_README = r"""DeskInfo - How to use config.cfg
================================

The "config.cfg" file defines which lines will be displayed by DeskInfo.
The FIRST TWO lines are inserted automatically by the program:
1) Machine name (nodename)
2) Fixed separator line ({SEPARATOR})

All other lines should be added to config.cfg using the variables below.
Example line in config.cfg:
    CPU: {cpu_usage} | RAM: {mem_used_gb}/{mem_total_gb}

Available variables:
----------------------
{machine_domain}   -> Domain name or WORKGROUP
{ip_address}       -> Primary IPv4 address (active interface)
{user_name}        -> Logged-in username
{logon_domain}     -> user@DOMAIN
{os_version}       -> Windows version (e.g., Windows 10 ...)
{system_type}      -> Architecture (e.g., AMD64)
{cpu_usage}        -> CPU usage (e.g., 12%)
{mem_used_gb}      -> Used memory (e.g., 3.2G)
{mem_total_gb}     -> Total memory (e.g., 8.0G)
{disk_c_usage}     -> Disk C: usage in percent (e.g., 65.2%)
{disk_info}        -> Free/total space for all drives (e.g., Free Space (C:): 120.4G of 500.0G)
{SEPARATOR}        -> Separator line (-------)

Tips:
------
- You can remove, modify, or reorder lines in config.cfg.
- To add new variables to the program, you need to edit the source code
  (extra variables will appear in the README once supported).
- If a variable does not exist, it will be displayed literally as {name}.
"""
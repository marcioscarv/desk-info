DeskInfo - How to use config.cfg
================================

The "config.cfg" file defines which lines will be displayed by DeskInfo.
Use Main_color to change the machine name and separators. Supported hexadecimal
formats are #RGB, #RGBA, #RRGGBB, and #RRGGBBAA:
    Main_color: #1E90FF
This setting line is not displayed. Invalid or missing values use #1E90FF.
Alpha values are accepted but only the RGB components are currently rendered.

The FIRST TWO lines are inserted automatically by the program:
1) Machine name (nodename)
2) Fixed separator line (--------------------------------------------------)

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
--------------------------------------------------        -> Separator line (-------)

Tips:
------
- You can remove, modify, or reorder lines in config.cfg.
- To add new variables to the program, you need to edit the source code
  (extra variables will appear in the README once supported).
- If a variable does not exist, it will be displayed literally as {name}.

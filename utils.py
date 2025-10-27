import os
import sys
import socket
import getpass
import platform
import psutil
from config import SEPARATOR, DEFAULT_CFG, DEFAULT_README

# Path setup for config and readme files
if getattr(sys, "frozen", False):
    BASE_DIR = os.path.dirname(sys.executable)
else:
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))

CFG_PATH = os.path.join(BASE_DIR, "config.cfg")
README_PATH = os.path.join(BASE_DIR, "readme.txt")

def ensure_files_exist():
    """Create config.cfg and readme.txt with default content if they don't exist."""
    if not os.path.exists(CFG_PATH):
        try:
            with open(CFG_PATH, "w", encoding="utf-8") as f:
                f.write(DEFAULT_CFG.replace("{SEPARATOR}", SEPARATOR))
        except Exception:
            pass

    if not os.path.exists(README_PATH):
        try:
            with open(README_PATH, "w", encoding="utf-8") as f:
                f.write(DEFAULT_README.replace("{SEPARATOR}", SEPARATOR))
        except Exception:
            pass

def load_config_lines():
    """Load lines from config.cfg (strip newlines)."""
    ensure_files_exist()
    try:
        with open(CFG_PATH, "r", encoding="utf-8") as f:
            return [ln.rstrip("\n") for ln in f.readlines() if ln.rstrip("\n") != ""]
    except Exception:
        return [ln for ln in DEFAULT_CFG.splitlines() if ln.strip() != ""]

class SafeDict(dict):
    """Dict that returns '{key}' for missing keys instead of raising KeyError."""
    def __missing__(self, key):
        return "{" + key + "}"

def get_primary_ip():
    """Find the primary IPv4 address from the active network interface."""
    try:
        # Obter estatísticas das interfaces de rede
        stats = psutil.net_if_stats()
        addrs = psutil.net_if_addrs()
        active_interfaces = []

        # Identificar interfaces ativas (up) com endereço IPv4
        for iface, iface_addrs in addrs.items():
            if iface in stats and stats[iface].isup:  # Verifica se a interface está ativa
                for addr in iface_addrs:
                    if getattr(addr, "family", None) == socket.AF_INET:
                        ip = getattr(addr, "address", "")
                        if ip and not ip.startswith("127."):  # Ignora loopback
                            # Priorizar interfaces com tráfego (bytes enviados/recebidos)
                            bytes_sent = stats[iface].bytes_sent if iface in stats else 0
                            bytes_recv = stats[iface].bytes_recv if iface in stats else 0
                            active_interfaces.append((ip, bytes_sent + bytes_recv))

        if active_interfaces:
            # Ordenar por tráfego (maior tráfego indica interface mais usada)
            active_interfaces.sort(key=lambda x: x[1], reverse=True)
            return active_interfaces[0][0]  # Retorna o IP da interface com maior tráfego

    except Exception:
        pass

    # Fallback: tentar obter IP via socket
    try:
        return socket.gethostbyname(socket.gethostname())
    except Exception:
        return "N/A"

def get_system_info_text():
    """Build the final text from system info and config.cfg."""
    nodename = socket.gethostname()
    user_name = getpass.getuser()
    ip_address = get_primary_ip()

    machine_domain = os.environ.get("USERDOMAIN", "N/A")
    if machine_domain and nodename and machine_domain.upper() == nodename.upper():
        machine_domain = "WORKGROUP"

    logon_domain = f"{user_name}@{machine_domain}"
    os_version = f"{platform.system()} {platform.release()} ({platform.version()})"
    system_type = platform.machine()
    cpu_usage = f"{psutil.cpu_percent(interval=0.1)}%"

    mem = psutil.virtual_memory()
    mem_used_gb = f"{mem.used / (1024**3):.1f}G"
    mem_total_gb = f"{mem.total / (1024**3):.1f}G"

    # Obter informações de todas as partições de disco
    disk_info = []
    disk_c_usage = "N/A"
    try:
        for partition in psutil.disk_partitions():
            try:
                disk = psutil.disk_usage(partition.mountpoint)
                disk_free_gb = f"{disk.free / (1024**3):.1f}G"
                disk_total_gb = f"{disk.total / (1024**3):.1f}G"
                disk_info.append(f"Free Space ({partition.mountpoint}): {disk_free_gb} of {disk_total_gb}")
                # Calcular uso do disco C: em percentual
                if partition.mountpoint == 'C:\\':
                    disk_c_usage = f"{disk.percent:.1f}%"
            except Exception:
                continue
    except Exception:
        disk_info.append("Free Space (C:): N/A of N/A")
        disk_c_usage = "N/A"

    data = {
        "nodename": nodename,
        "machine_domain": machine_domain,
        "ip_address": ip_address,
        "user_name": user_name,
        "logon_domain": logon_domain,
        "os_version": os_version,
        "system_type": system_type,
        "cpu_usage": cpu_usage,
        "mem_used_gb": mem_used_gb,
        "mem_total_gb": mem_total_gb,
        "disk_info": "\n".join(disk_info),  # Todas as partições
        "disk_c_usage": disk_c_usage,  # Uso do disco C: em percentual
        "SEPARATOR": SEPARATOR
    }

    lines = load_config_lines()
    formatted_lines = []
    for ln in lines:
        try:
            formatted_lines.append(ln.format_map(SafeDict(**data)))
        except Exception:
            formatted_lines.append(ln)

    final_text = nodename + "\n" + SEPARATOR + "\n" + "\n".join(formatted_lines)
    return final_text
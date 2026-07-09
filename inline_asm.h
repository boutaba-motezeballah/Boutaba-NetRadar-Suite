import sys
import subprocess

def get_live_hardware_networks():
    networks = []
    try:
        if sys.platform.startswith("win"):
            cmd = "netsh wlan show networks mode=bssid"
            stdout = subprocess.check_output(cmd, shell=True).decode('utf-8', errors='ignore')
            lines = stdout.split('\n')
            current_ssid, current_bssid, current_signal, current_cipher = "", "", "", ""
            
            for line in lines:
                line = line.strip()
                if line.startswith("SSID"):
                    parts = line.split(":")
                    if len(parts) > 1: 
                        # Fix: Dynamic string splitting to catch raw SSID after the colon
                        current_ssid = ":".join(parts[1:]).strip()
                elif line.startswith("Authentication"):
                    parts = line.split(":")
                    if len(parts) > 1: current_cipher = parts[1:].strip()
                elif line.startswith("BSSID 1"):
                    parts = line.split(":")
                    if len(parts) > 1: current_bssid = ":".join(parts[1:]).strip()
                elif line.startswith("Signal"):
                    parts = line.split(":")
                    if len(parts) > 1:
                        current_signal = parts[1:].strip()
                        if current_ssid and current_bssid:
                            networks.append({
                                "ssid": current_ssid[:21] if current_ssid else "Hidden_SSID",
                                "bssid": current_bssid,
                                "signal": current_signal,
                                "freq": "Dynamic Link",
                                "cipher": current_cipher[:13] if current_cipher else "WPA2"
                            })
                            current_ssid, current_bssid = "", ""
        else:
            # Fix: Utilizing customized delimiter options on nmcli payload parsing
            cmd = "nmcli -t -f SSID,BSSID,SIGNAL,SECURITY,CHAN dev wifi list"
            stdout = subprocess.check_output(cmd, shell=True).decode('utf-8', errors='ignore')
            lines = stdout.strip().split('\n')
            for line in lines:
                parts = line.split(':')
                if len(parts) >= 5:
                    # Capturing absolute cell structures correctly over multi-byte names
                    ssid = parts[0] if parts[0] != "" else "Hidden_SSID"
                    bssid = ":".join(parts[1:7])
                    signal = parts[7] + "%"
                    cipher = parts[8]
                    chan = parts[9]
                    networks.append({
                        "ssid": ssid[:21],
                        "bssid": bssid,
                        "signal": signal,
                        "freq": f"CH {chan}",
                        "cipher": cipher[:13]
                    })
    except Exception:
        pass
        
    return networks

def analyze_best_network(networks):
    if not networks:
        return "None_Detected", "Hardware scanner returned empty set. No physical signals active."
        
    best_net = None
    max_sig = -1
    
    for net in networks:
        try:
            sig_val = int(net["signal"].replace("%", "").replace("dBm", "").strip())
        except Exception:
            sig_val = 0
        if sig_val > max_sig:
            max_sig = sig_val
            best_net = net
            
    if best_net:
        reason = f"Highest live signal metrics verified at {best_net['signal']} via channel {best_net['freq']}."
        return best_net["ssid"], reason
    return "None_Detected", "Hardware link scan status failed to capture stable telemetry data."

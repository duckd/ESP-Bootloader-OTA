
import datetime
version = "v1.0.0"
today = datetime.date.today().isoformat()
with open("include/version.h", "w") as f:
    f.write(f'#define BUILD_VERSION "{version} ({today})"\n')

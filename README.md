**<sup>This repository is a fork of [sw1tchbl4d3's rdo](https://codeberg.org/sw1tchbl4d3/rdo)</sup>**

# RootDo

This project aims to be a very slim alternative to both sudo and doas.

### Usage

```
rdo [command]
```

### Installation

If you are on Arch Linux, you can download the package via the [AUR](https://aur.archlinux.org/packages/rdo/).

If you are using any other Linux distro, or want to build it yourself, you will first need to install either `libbsd` or `libbsd-dev`, depending on how your package manager calls it.

Then, as the root user, build `rdo` with the following command:

```
make && make install
```

After that, you'll have to configure `rdo` to allow you to use it.
To do this, edit `/etc/rdo.conf` and you're good to go!

The configuration file has the following variables:
```
username=<username>
wrong_pw_sleep=<milliseconds>
session_ttl=<minutes>
```

- `username`: The username of the user that is allowed to execute rdo (no multi user or group support (yet)).
- `wrong_pw_sleep`: The amount of milliseconds to sleep at a wrong password attempt. Must be a positive integer. Set to 0 to disable.
- `session_ttl`: The amount of minutes a session lasts. Must be a positive integer. Set to 0 to disable.

#### To uninstall:
```
make uninstall
```

### Benchmarks

The benchmark: Execute `whoami` (GNU coreutils 8.32) 1000 times.

| Program        | Time   |
|----------------|--------|
| sudo 1.9.7p1   | 13.62s |
| opendoas 6.8.1 | 7.60s  |
| rdo 1.2        | 2.25s  |
| root user[^1]  | 1.43s  |

[^1]: Executing the command just as root without any utility to execute the command as root.

These benchmarks were done on a single core of an `Intel i3-3110M` Laptop processor, on Artix Linux version `5.13.4-zen2-1-zen`.

`sudo` and `opendoas` were pulled from the pacman repos, rdo via AUR.

All configs were kept as default, except allow the `wheel` group on both + enable `persist` on doas.

Script used:
```sh
#!/bin/sh

$1 whoami

current=$(date +%s.%N)
for i in {1..1000}; do
	$1 whoami 2>&1 >/dev/null
done
done=$(date +%s.%N)

echo $done - $current | bc
```

The script requires `bc` to be installed, for floating point arithmetics.

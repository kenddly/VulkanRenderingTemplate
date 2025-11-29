conan_install_linux:
	conan install . -pr linux_release --build=missing

conan_install_windows:
	conan install . -pr windows_release --build=missing

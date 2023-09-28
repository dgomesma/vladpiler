FROM archlinux:base

RUN pacman -Syu --noconfirm 
RUN pacman -S --noconfirm reflector
RUN reflector --latest 5 --sort rate --protocol http,https --save /etc/pacman.d/mirrorlist
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm llvm clang gcc flex bison make cxxopts boost

RUN mkdir /usr/src/vladpiler
WORKDIR /usr/src/vladpiler

COPY . .
RUN make

RUN mkdir /var/rinha
CMD ["/usr/src/vladpiler/run.sh", "/var/rinha/source.rinha"]
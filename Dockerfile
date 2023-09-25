FROM archlinux:base

RUN pacman -Syu --noconfirm 
RUN pacman -S --noconfirm llvm clang gcc flex bison make cxxopts boost

WORKDIR /home/user/vladpiler
COPY . .

RUN make
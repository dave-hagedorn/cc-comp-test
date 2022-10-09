ARG TAG=20.04

FROM ubuntu:$TAG

ARG USER=dev
ENV HOME=/home/$USER
ENV PATH=$PATH:$HOME/bin:$HOME/.local/bin

RUN apt update && apt install -y \
    zsh \
    sudo

RUN useradd --user-group --shell /usr/bin/zsh --create-home --home-dir $HOME --password dev  dev
RUN echo "dev ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# SW needed for rest of Dockerfile
RUN apt install -y \
    wget

USER $USER

RUN mkdir $HOME/bin

RUN wget https://github.com/bazelbuild/bazelisk/releases/download/v1.11.0/bazelisk-linux-amd64 -O $HOME/bin/bazel \
    && chmod +x $HOME/bin/bazel

# These are mounted at container start, pre-creating the path sets the permissions
# on these mounts
RUN for path in \
    "$HOME" \
    "$HOME/.vscode-server/extensions" \
    "$HOME/.vscode-server-insiders/extensions" \
    "$HOME/.config" \
    "$HOME/.cache"; \
    do \
    [[ -d "$path" ]] || mkdir -p "$path"; \
    chown $USER:$USER -R "$path"; \
    done

# ibazel watcher
RUN wget https://github.com/bazelbuild/bazel-watcher/releases/download/v0.15.10/ibazel_linux_amd64 \
    -O $HOME/bin/ibazel \
    && chmod +x $HOME/bin/ibazel

# Some Bazel tools expect python, not python3
RUN sudo ln -s /usr/bin/python3 /usr/bin/python

# Run clang static analyzer and stuff
RUN pip install codechecker

# Buildifier - Bazel file formatter
RUN wget https://github.com/bazelbuild/buildtools/releases/download/4.2.5/buildifier-linux-amd64 \
    -O $HOME/bin/buildifier \
    && chmod +x $HOME/bin/buildifier


WORKDIR /home/$USER

# Do last so this can be updated as needed
# This needs to be copied into the container instead of assumed accessible via the usual workspace bind mount
# Those using devconainers on Windows/Mac may want to volume mount the workspace because bind mounts are so slow there
COPY --chown=$USER:$USER .devcontainer/post-start.sh /home/$USER/bin/
RUN chmod +x $HOME/bin/*


# SW needed by dev or build
# put last so easier to update in Dockerfile - the above should change rarely
# git           ...
# python        needed by some Bazel tooling and 3rd party tools
# libc6-dev     libc - needed by libc++, ... everything
# binutils      ld (not in bazel toolchain)
# gcc           crtbegin.o, other startup libs
# libc++-dev    libc++.s0.1 - system's libc++ runtime (Bazel toolchain doesn't include this?)
# nano          git editor
# tig           git history
#
# could exclude in final layers if optimizing size
# pip           to install codechecker (could exclude in final layer)
# wget          download stuff
RUN sudo apt install -y \
    git \
    python3 \
    libc6-dev \
    wget \
    binutils \
    gcc \
    libc++-dev \
    pip \
    nano \
    tig \
    jq
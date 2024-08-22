FROM espressif/idf

ARG DEBIAN_FRONTEND=noninteractive
ARG CONTAINER_USER=esp
ARG USER_UID=1050
ARG USER_GID=$USER_UID

RUN echo "Set disable_coredump false" >> /etc/sudo.conf

RUN apt update -q

RUN apt-get update \
  && apt install -y -q \
  cmake \
  git \
  libglib2.0-0 \
  libnuma1 \
  libpixman-1-0 \
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
  && apt install -y -q -yq \
  sudo --option=Dpkg::Options::=--force-confdef\
  lsb-release \
  gosu \
  nano \
  && rm -rf /var/lib/apt/lists/*

COPY ./install_micro_ros_deps_script.sh /install_micro_ros_deps_script.sh

RUN mkdir -p /tmp/install_micro_ros_deps_script && mv /install_micro_ros_deps_script.sh /tmp/install_micro_ros_deps_script/ && \
    IDF_EXPORT_QUIET=1 /tmp/install_micro_ros_deps_script/install_micro_ros_deps_script.sh && \
    rm -rf /var/lib/apt/lists/*

RUN /usr/bin/pip3 --no-cache-dir install --break-system-packages catkin_pkg lark-parser colcon-common-extensions importlib-resources

# QEMU
ENV QEMU_REL=esp_develop_8.2.0_20240122
ENV QEMU_SHA256=e7c72ef5705ad1444d391711088c8717fc89f42e9bf6d1487f9c2a326b8cfa83
ENV QEMU_DIST=qemu-xtensa-softmmu-${QEMU_REL}-x86_64-linux-gnu.tar.xz
ENV QEMU_URL=https://github.com/espressif/qemu/releases/download/esp-develop-8.2.0-20240122/${QEMU_DIST}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN wget --no-verbose ${QEMU_URL} \
  && echo "${QEMU_SHA256} *${QEMU_DIST}" | sha256sum --check --strict - \
  && tar -xf $QEMU_DIST -C /opt \
  && rm ${QEMU_DIST}

ENV PATH=/opt/qemu/bin:${PATH}

RUN groupadd --gid $USER_GID $CONTAINER_USER \
  && adduser --uid $USER_UID --gid $USER_GID --disabled-password --gecos "" ${CONTAINER_USER} \
  && usermod -a -G root $CONTAINER_USER && usermod -a -G dialout $CONTAINER_USER

RUN chmod -R 775 /opt/esp/python_env/

USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc
ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]
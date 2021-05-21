FROM dockcross/web-wasm:20200119-1c10fb2

RUN apt-get update \
    && apt-get install -y \
    gfortran \
    libc6-dev-i386

WORKDIR /opt/kaldi
RUN git clone -b lookahead --single-branch https://github.com/alphacep/kaldi . && \
    git checkout 89bc31d

WORKDIR /opt/kaldi/tools/CLAPACK-WA
RUN git clone https://github.com/adrianbg/CLAPACK-WA .
ADD CLAPACK-WA.patch /tmp
RUN git apply -v /tmp/CLAPACK-WA.patch
RUN emmake make -j $(nproc)

WORKDIR /opt/kaldi/tools/gsl
RUN curl --fail -q -L http://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.6.tar.gz | \
    tar xz --strip-components=1
RUN emconfigure ./configure
RUN emmake make -j $(nproc)

WORKDIR /opt
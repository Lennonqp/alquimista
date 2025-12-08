FROM python:3.9-slim
RUN apt-get update && apt-get install -y gcc
WORKDIR /app
COPY . .
# Este comando cria o arquivo .so que o servidor precisa
RUN gcc -shared -o backend.so -fPIC backend.c
RUN pip install flask gunicorn
CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:10000", "app:app"]

from flask import Flask, render_template, session, redirect, url_for
import ctypes
import os
import sys

app = Flask(__name__)
app.secret_key = 'alquimista' # Necessário para guardar o histórico do usuário

# --- 1. CONFIGURAÇÃO DO BACKEND C ---
pasta_script = os.path.dirname(os.path.abspath(__file__))
os.chdir(pasta_script)

if os.name == 'nt':
    lib_name = "backend.dll"
else:
    lib_name = "./backend.so"

try:
    backend = ctypes.CDLL(os.path.join(pasta_script, lib_name))
    # Configurar tipos
    backend.interagir_arvore.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    backend.interagir_arvore.restype = None
except OSError:
    print("ERRO: Não encontrei o backend_arvore.dll. Compile o código C primeiro!")
    sys.exit()

def consultar_c(caminho):
    buffer_resp = ctypes.create_string_buffer(4096)
    caminho_bytes = caminho.encode('utf-8')
    backend.interagir_arvore(caminho_bytes, buffer_resp)
    return buffer_resp.value.decode('utf-8', errors='ignore')

# --- 2. ROTAS DO SITE ---

@app.route('/')
def index():
    # Se é a primeira vez, inicia o caminho vazio
    if 'caminho' not in session:
        session['caminho'] = ""

    # Pergunta ao C o estado atual
    resposta_raw = consultar_c(session['caminho'])
    
    dados = {}
    
    # Processa a resposta para enviar ao HTML
    if resposta_raw.startswith("PERGUNTA:"):
        dados['tipo'] = 'pergunta'
        dados['texto'] = resposta_raw.replace("PERGUNTA:", "")
        
    elif resposta_raw.startswith("RESULTADO:"):
        dados['tipo'] = 'resultado'
        # Formata o texto para HTML (quebras de linha)
        texto_resultado = resposta_raw.replace("RESULTADO:", "")
        dados['lista'] = texto_resultado.split("\n")
        
    elif resposta_raw.startswith("FIM:"):
        dados['tipo'] = 'fim'
        
    return render_template('index.html', dados=dados)

@app.route('/responder/<escolha>')
def responder(escolha):
    # Escolha é '1' (Sim) ou '0' (Não)
    if 'caminho' in session:
        session['caminho'] += escolha
    return redirect(url_for('index'))

@app.route('/reiniciar')
def reiniciar():
    session['caminho'] = ""
    return redirect(url_for('index'))

if __name__ == '__main__':
    # Debug=True faz o site recarregar se mudares o código
    app.run(debug=True)
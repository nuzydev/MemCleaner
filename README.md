# MemCleaner
Ferramenta C++ para Windows que localiza e substitui strings sensíveis na memória de processos. Remove credenciais, tokens e dados confidenciais em tempo real.

---

## ⚙️ Funcionalidades

- 🔍 Busca por strings (case-insensitive) em memória de processos e serviços
- 🧼 Sobrescreve as strings diretamente na memória usando `WriteProcessMemory`
- 🛡️ Suporte a `SeDebugPrivilege` para lidar com processos protegidos
- 🧩 CLI simples e funcional

---

## 📥 Instalação

### 🧰 Pré-requisitos
- Compilador C++ (g++, clang++ ou MSVC)
- Sistema Operacional: **Windows**

### 🏗️ Compilando com g++ (MinGW)

g++ main.cpp memory_cleaner.cpp -o MemCleaner.exe -static -lpsapi

Ou via Visual Studio:
- Crie um projeto C++ Console
- Adicione `main.cpp` e `memory_cleaner.cpp` ao projeto
- Compile em Release x64

---

## 🚀 Como Usar

MemCleaner.exe --process <ProcessName.exe> <string_a_limpar>  
MemCleaner.exe --service <ServiceName> <string_a_limpar>

### 📌 Exemplos:

MemCleaner.exe --process notepad.exe senha123  
MemCleaner.exe --service Spooler token_do_sistema

---

## ⚠️ Permissões

- O programa habilita o privilégio `SeDebugPrivilege` automaticamente.
- Execute como **administrador** para acessar processos/serviços restritos.

---

## 🔐 Aviso Legal

> Este projeto tem fins **educacionais** e de **auditoria de segurança**. O uso indevido é de responsabilidade única do usuário.

---

## 🧠 Como Funciona

1. Busca o PID do processo ou serviço fornecido  
2. Escaneia a memória virtual do processo  
3. Encontra todas as ocorrências da string (insensível a maiúsculas/minúsculas)  
4. Sobrescreve as ocorrências com bytes nulos (`\0`)

---

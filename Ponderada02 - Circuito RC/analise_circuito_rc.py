#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Análise de Circuito RC - Ponderada 02
M4-IOT - Professor Bryan Kano

Este script analisa os dados coletados do circuito RC e gera gráficos
de carga no capacitor, descarga no resistor e comparação dos comportamentos.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import rcParams

# Configurações para melhorar a aparência dos gráficos
rcParams['font.family'] = 'DejaVu Sans'
rcParams['font.size'] = 10
rcParams['axes.grid'] = True
rcParams['grid.alpha'] = 0.3
rcParams['figure.facecolor'] = 'white'
rcParams['axes.facecolor'] = 'white'

def carregar_dados(tipo='simulados'):
    """Carrega os dados do arquivo CSV"""
    arquivo = 'valores_simulados.csv' if tipo == 'simulados' else 'valores_reais.csv'
    try:
        df = pd.read_csv(arquivo)
        print(f"Dados {tipo} carregados com sucesso!")
        print(f"Total de pontos: {len(df)}")
        print(f"Tempo: {df['Tempo_ms'].min():.0f}ms a {df['Tempo_ms'].max():.0f}ms")
        return df
    except FileNotFoundError:
        print(f"Arquivo '{arquivo}' nao encontrado!")
        return None
    except Exception as e:
        print(f"Erro ao carregar dados: {e}")
        return None

def plotar_carga_capacitor(df, tipo='simulados'):
    """Gera gráfico da carga no capacitor"""
    plt.figure(figsize=(10, 6))
    
    # Usar todos os dados coletados
    plt.plot(df['Tempo_ms'], df['Tensao_Capacitor_V'], 
             'b-', linewidth=2, label='Tensão no C (Vc)')
    
    titulo = f'Carga no Capacitor (C) - {tipo.title()}'
    plt.title(titulo, fontsize=14, fontweight='bold')
    plt.xlabel('Tempo (ms)', fontsize=12)
    plt.ylabel('Tensão (V)', fontsize=12)
    plt.xlim(0, df['Tempo_ms'].max())
    plt.ylim(0.0, 5.0)  # Capacitor vai de 0V a ~4.3V
    
    # Configurar ticks do eixo X baseado nos dados reais
    max_time = int(df['Tempo_ms'].max())
    step = max_time // 10  # 10 divisões aproximadamente
    plt.xticks(np.arange(0, max_time + step, step))
    plt.yticks(np.arange(0.0, 5.5, 0.5))
    
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    # Salvar gráfico com nome específico
    nome_arquivo = f'grafico_carga_capacitor_{tipo}.png'
    plt.savefig(nome_arquivo, dpi=300, bbox_inches='tight')
    print(f"Grafico de carga no capacitor ({tipo}) salvo!")
    plt.show()

def plotar_descarga_resistor(df, tipo='simulados'):
    """Gera gráfico da descarga no resistor"""
    plt.figure(figsize=(10, 6))
    
    # Usar todos os dados coletados
    plt.plot(df['Tempo_ms'], df['Tensao_Resistor_V'], 
             'r-', linewidth=2, label='Tensão no R (Vr)')
    
    titulo = f'Descarga no Resistor (R) - {tipo.title()}'
    plt.title(titulo, fontsize=14, fontweight='bold')
    plt.xlabel('Tempo (ms)', fontsize=12)
    plt.ylabel('Tensão (V)', fontsize=12)
    plt.xlim(0, df['Tempo_ms'].max())
    plt.ylim(0.0, 5.5)  # Resistor vai de 5V a ~0.7V
    
    # Configurar ticks do eixo X baseado nos dados reais
    max_time = int(df['Tempo_ms'].max())
    step = max_time // 10  # 10 divisões aproximadamente
    plt.xticks(np.arange(0, max_time + step, step))
    plt.yticks(np.arange(0.0, 6.0, 0.5))
    
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    # Salvar gráfico com nome específico
    nome_arquivo = f'grafico_descarga_resistor_{tipo}.png'
    plt.savefig(nome_arquivo, dpi=300, bbox_inches='tight')
    print(f"Grafico de descarga no resistor ({tipo}) salvo!")
    plt.show()

def plotar_comparacao(df, tipo='simulados'):
    """Gera gráfico de comparação entre carga e descarga"""
    plt.figure(figsize=(10, 6))
    
    # Usar todos os dados coletados
    plt.plot(df['Tempo_ms'], df['Tensao_Capacitor_V'], 
             'b-', linewidth=2, label='Tensão no C (Vc)')
    plt.plot(df['Tempo_ms'], df['Tensao_Resistor_V'], 
             'r-', linewidth=2, label='Tensão no R (Vr)')
    
    titulo = f'Comparação: Carga no C e Descarga no R - {tipo.title()}'
    plt.title(titulo, fontsize=14, fontweight='bold')
    plt.xlabel('Tempo (ms)', fontsize=12)
    plt.ylabel('Tensão (V)', fontsize=12)
    plt.xlim(0, df['Tempo_ms'].max())
    plt.ylim(0, 5.5)
    
    # Configurar ticks do eixo X baseado nos dados reais
    max_time = int(df['Tempo_ms'].max())
    step = max_time // 10  # 10 divisões aproximadamente
    plt.xticks(np.arange(0, max_time + step, step))
    plt.yticks(np.arange(0, 6, 1))
    
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    # Salvar gráfico com nome específico
    nome_arquivo = f'grafico_comparacao_{tipo}.png'
    plt.savefig(nome_arquivo, dpi=300, bbox_inches='tight')
    print(f"Grafico de comparacao ({tipo}) salvo!")
    plt.show()

def analisar_dados(df):
    """Realiza análise estatística dos dados"""
    print("\nANALISE DOS DADOS:")
    print("=" * 50)
    
    # Análise da tensão do capacitor
    vc_inicial = df['Tensao_Capacitor_V'].iloc[0]
    vc_final = df['Tensao_Capacitor_V'].iloc[-1]
    vc_max = df['Tensao_Capacitor_V'].max()
    
    print(f"Capacitor:")
    print(f"   Tensao inicial: {vc_inicial:.2f} V")
    print(f"   Tensao final: {vc_final:.2f} V")
    print(f"   Tensao maxima: {vc_max:.2f} V")
    print(f"   Variacao: {vc_final - vc_inicial:.2f} V")
    
    # Análise da tensão do resistor
    vr_inicial = df['Tensao_Resistor_V'].iloc[0]
    vr_final = df['Tensao_Resistor_V'].iloc[-1]
    vr_max = df['Tensao_Resistor_V'].max()
    
    print(f"\nResistor:")
    print(f"   Tensao inicial: {vr_inicial:.2f} V")
    print(f"   Tensao final: {vr_final:.2f} V")
    print(f"   Tensao maxima: {vr_max:.2f} V")
    print(f"   Variacao: {vr_final - vr_inicial:.2f} V")
    
    # Verificação da Lei de Kirchhoff
    soma_inicial = vc_inicial + vr_inicial
    soma_final = vc_final + vr_final
    
    print(f"\nLei de Kirchhoff:")
    print(f"   Soma inicial (Vc + Vr): {soma_inicial:.2f} V")
    print(f"   Soma final (Vc + Vr): {soma_final:.2f} V")
    print(f"   Tensao da fonte: 5.00 V")
    
    if abs(soma_inicial - 5.0) < 0.1 and abs(soma_final - 5.0) < 0.1:
        print("   Lei de Kirchhoff verificada!")
    else:
        print("   Discrepancia na Lei de Kirchhoff")

def main():
    """Função principal"""
    print("ANALISE DE CIRCUITO RC - PONDERADA 02")
    print("=" * 50)
    
    # Carregar dados reais
    df_reais = carregar_dados('reais')
    if df_reais is None:
        return
    
    # Analisar dados reais
    print("\n=== DADOS FISICOS (HARDWARE REAL) ===")
    analisar_dados(df_reais)
    
    # Gerar gráficos dos dados reais
    print("\nGERANDO GRAFICOS DOS DADOS REAIS:")
    print("=" * 40)
    
    plotar_carga_capacitor(df_reais, 'reais')
    plotar_descarga_resistor(df_reais, 'reais')
    plotar_comparacao(df_reais, 'reais')
    
    print("\nANALISE DOS DADOS REAIS CONCLUIDA!")
    print("Graficos salvos:")
    print("   - grafico_carga_capacitor_reais.png")
    print("   - grafico_descarga_resistor_reais.png")
    print("   - grafico_comparacao_reais.png")

if __name__ == "__main__":
    main()

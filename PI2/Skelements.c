#include<stdio.h>
#include<allegro5/allegro.h>
#include<allegro5/allegro_primitives.h>
#include<stdbool.h>
#include<allegro5/keyboard.h>
#include<allegro5/mouse.h>
#include<allegro5/timer.h>
#include<allegro5/allegro_font.h>
#include<allegro5/allegro_ttf.h>
#include <stdlib.h>
#include <time.h>
#include<allegro5/allegro_image.h>

typedef struct {
	float x;
	float y;
	float raio;
	float velocidade;
	bool ativo;
	int dano;
	int vida;
}inimigo;

typedef struct {
	float xi;
	float xf;
	float yi;
	float yf;
	float velocidade;
	float velocidadePulo;
	bool noChao;
	bool virou;
	bool ativo;
	int dano;
	int vida;
}Personagem;

void movimentacao(Personagem* p, int largura_tela) {

	al_install_keyboard();
	ALLEGRO_KEYBOARD_STATE teclado;
	al_get_keyboard_state(&teclado);

	//variaveis
	p->velocidade = 0.03f;
	float gravidade = 0.015f;
	//

// Movimentação para a esquerda
	if (al_key_down(&teclado, ALLEGRO_KEY_LEFT) && p->ativo == true) {
		p->xi -= p->velocidade;
		p->xf -= p->velocidade;
		p->virou = true;
	}

	// Movimentação para a direita
	if (al_key_down(&teclado, ALLEGRO_KEY_RIGHT) && p->ativo == true) {
		p->xi += p->velocidade;
		p->xf += p->velocidade;
		p->virou = false;
	}
	// Impede que o personagem saia da tela
	if (p->xi < 0) {
		p->xi = 0;
		p->xf = 50;
	}

	if (p->xf > largura_tela) {
		p->xf = largura_tela;
		p->xi = largura_tela - 50;
	}

	// Pulo
	if (al_key_down(&teclado, ALLEGRO_KEY_UP) && p->noChao == true && p->ativo == true) {
		p->velocidadePulo = -1.8f;
		p->noChao = false;
	}

	p->velocidadePulo += gravidade;
	//gravidade
	p->yi += p->velocidadePulo;
	p->yf += p->velocidadePulo;

	// Colisão com o chão
	if (p->yf >= 400) {
		p->yf = 400;
		p->yi = p->yf - 80;
		p->velocidadePulo = 0.0f;
		p->noChao = true;
	}
}

void  projetil(float* balaX, float* balaY, bool* balaAtiva, float xi, float xf, float yi, float yf, int largura_tela, bool* mouseAnterior, bool* virou, bool* balaVirou, bool ativoP) {

	al_install_mouse();
	ALLEGRO_MOUSE_STATE mouse;
	al_get_mouse_state(&mouse);

	bool mouseAtual = al_mouse_button_down(&mouse, ALLEGRO_MOUSE_BUTTON_LEFT);

	float velobala = 0.05f;

	// Detecta um novo clique
	if (mouseAtual && !*mouseAnterior && !*balaAtiva && ativoP == true) {
		*balaVirou = *virou;

		if (*balaVirou == false) {
			*balaX = xf;
		}
		else {
			*balaX = xi;
		}
		*balaY = yi + (yf - yi) / 2;
		*balaAtiva = true;
	}

	*mouseAnterior = mouseAtual;

	if (*balaAtiva && *balaVirou == false && ativoP == true) {
		*balaX += velobala;

		al_draw_filled_rectangle(
			*balaX, *balaY - 5,
			*balaX + 20, *balaY + 5,
			al_map_rgb(0, 255, 0)
		);//projetil desenhado direita
	}
	else
	{
		if (*balaAtiva && *balaVirou == true && ativoP == true) {
			*balaX -= velobala;
			al_draw_filled_rectangle(
				*balaX, *balaY - 5,
				*balaX - 20, *balaY + 5,
				al_map_rgb(0, 255, 0)
			);//projetil desenhado esquerda
		}
	}

	// Desativa a bala quando sair da tela
	if (*balaX > largura_tela) {
		*balaAtiva = false;
	}
	else {
		if (*balaX < 0) {
			*balaAtiva = false;
		}
	}
}

void colisaoBala(float* balaX, float* balaY, bool* balaAtiva, bool* balaVirou, inimigo* inimigo) {

	if (*balaAtiva && inimigo->ativo) {

		float balaEsquerda;
		float balaDireita;

		// Define os limites horizontais da bala
		if (*balaVirou == false) {
			balaEsquerda = *balaX;
			balaDireita = *balaX + 20;
		}
		else {
			balaEsquerda = *balaX - 20;
			balaDireita = *balaX;
		}

		float balaTopo = *balaY - 5;
		float balaBaixo = *balaY + 5;

		// Limites do inimigo
		float inimigoEsquerda = inimigo->x - inimigo->raio;
		float inimigoDireita = inimigo->x + inimigo->raio;

		// Verifica colisão
		if (balaDireita >= inimigoEsquerda && balaEsquerda <= inimigoDireita) {
			inimigo->vida -= 1;
			*balaAtiva = false;
		}
	}

	// Verifica se o inimigo morreu
	if (inimigo->vida <= 0) {
		inimigo->ativo = false;
	}
}

void colisaoPrincipal(Personagem* p, inimigo* inimigo) {

	if (!inimigo->ativo) {
		return;
	}
	// Ponto mais próximo do círculo dentro do retângulo
	float pontoX = inimigo->x;
	float pontoY = inimigo->y;

	if (pontoX < p->xi) {
		pontoX = p->xi;
	}
	else if (pontoX > p->xf) {
		pontoX = p->xf;
	}
	if (pontoY < p->yi) {
		pontoY = p->yi;
	}
	else if (pontoY > p->yf) {
		pontoY = p->yf;
	}

	// Distância entre o círculo e o ponto
	float dx = inimigo->x - pontoX;
	float dy = inimigo->y - pontoY;

	float distancia = dx * dx + dy * dy;

	// Colisão
	if (distancia < inimigo->raio * inimigo->raio &&
		p->yf > inimigo->y - inimigo->raio) {

		// Verifica se o personagem está pisando no inimigo
		if (p->yf < inimigo->y) {
			return;
		}
		//dano no personagem principal
		p->vida -= inimigo->dano;

		// Empurra o inimigo para fora do personagem
		if (inimigo->x < (p->xi + p->xf) / 2)
			inimigo->x = p->xi - inimigo->raio;
		else
			inimigo->x = p->xf + inimigo->raio;
	}
}

void perseguir_jogador(inimigo* inimigo, float jogadorX) {

	if (!inimigo->ativo)
		return;

	// Para a direita
	if (inimigo->x < jogadorX) {
		inimigo->x += inimigo->velocidade;
	}

	// Para a esquerda
	else if (inimigo->x > jogadorX) {
		inimigo->x -= inimigo->velocidade;
	}
}


int main() {
	//inicializando allegro
	al_init();
	al_init_primitives_addon();
	al_install_keyboard();
	al_install_mouse();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_image_addon();
	//

	//display dimensões (tela)
	int altura_tela = 500;
	int largura_tela = 860;
	ALLEGRO_DISPLAY* display = al_create_display(largura_tela, altura_tela);
	al_set_window_position(display, 200, 200);
	//

		//variaveis	
	ALLEGRO_BITMAP* menuImg = al_load_bitmap("Imagens/MenuImagem.png");
	Personagem principal = { 400.0f, 450.0f, 320.0f, 400.0f, 0.03f, 0.0f, true, false, true, 1 ,30 };
	float balaX = 0.0f;
	float balaY = 0.0f;
	bool balaAtiva = false;
	bool balaVirou = false;
	ALLEGRO_KEYBOARD_STATE teclado;
	ALLEGRO_MOUSE_STATE mouse;
	bool mouseAnterior = false;
	ALLEGRO_FONT* fonte = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 24, 0);
	bool menu = true;
	bool laboratorio = false;
	// variaveis Waves



	//
	//struct inimigo
	inimigo inimigo[3] = {
		{-30, 370,30,0.02f,true, 5, 3}, // azul
		{-40, 370,30,0.02f, true, 5, 3}, // branco
		{-50, 370,30,0.02f, true, 5, 3}  // amarelo
	};
	//randomizar local de spawn dos inimigos
	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		if (rand() % 2 == 0) {
			inimigo[i].x = -inimigo[i].raio;
		}
		else {
			inimigo[i].x = largura_tela + inimigo[i].raio;
		}
	}
	//


	while (true) {
		al_get_keyboard_state(&teclado);
		al_get_mouse_state(&mouse);

		if (menu == true) {
			al_draw_scaled_bitmap(menuImg, 0, 0, al_get_bitmap_width(menuImg), al_get_bitmap_height(menuImg), 0, 0, largura_tela, altura_tela, 0);
			al_draw_rectangle(340, 259, 520, 299, al_map_rgb(255, 0, 0), 5);
			al_draw_rectangle(340, 365, 520, 420, al_map_rgb(255, 0, 0), 5);
			if (al_mouse_button_down(&mouse, ALLEGRO_MOUSE_BUTTON_LEFT) && mouse.x >= 340 && mouse.x <= 520 && mouse.y >= 259 && mouse.y <= 299) {
				menu = false;
				laboratorio = false;
			}
			else if (al_mouse_button_down(&mouse, ALLEGRO_MOUSE_BUTTON_LEFT) && mouse.x >= 340 && mouse.x <= 520 && mouse.y >= 365 && mouse.y <= 420) {
				laboratorio = true;


			}
		}
		else if (menu == false) {
			//chamando a função
			movimentacao(&principal, largura_tela);
			// Calcula o centro horizontal do jogador
			float jogadorCentroX = (principal.xi + principal.xf) / 2;
			// Atualiza a posição dos inimigos
			perseguir_jogador(&inimigo[0], jogadorCentroX);
			perseguir_jogador(&inimigo[1], jogadorCentroX);
			perseguir_jogador(&inimigo[2], jogadorCentroX);

			//colisao do personagem principal
			colisaoPrincipal(&principal, &inimigo[0]);
			colisaoPrincipal(&principal, &inimigo[1]);
			colisaoPrincipal(&principal, &inimigo[2]);
			//colisao do projetil //float* balaX,float* balaY,bool* balaAtiva,bool* balaVirou,inimigo* inimigo)
			colisaoBala(&balaX, &balaY, &balaAtiva, &balaVirou, &inimigo[0]);
			colisaoBala(&balaX, &balaY, &balaAtiva, &balaVirou, &inimigo[1]);
			colisaoBala(&balaX, &balaY, &balaAtiva, &balaVirou, &inimigo[2]);



			al_clear_to_color(al_map_rgb(194, 191, 29));

			if (fonte == NULL) {
				printf("Erro ao carregar fonte\n");
				return -1;
			}
			if (principal.vida < 0) {
				principal.vida = 0;
			}
			// texto da vida do personagem
			al_draw_textf(fonte, al_map_rgb(255, 255, 255), 10, 10, 0, "Vida: %d", principal.vida);


			if (principal.vida > 0) {
				principal.ativo = true;
				al_draw_filled_rectangle(principal.xi, principal.yi, principal.xf, principal.yf, al_map_rgb(255, 0, 0));//personagem

			}
			else {
				principal.ativo = false;
			}

			//função projetil float *balaX, float *balaY, bool *balaAtiva, float xf, float yi, float yf, int largura_tela, bool *mouseAnterior, bool *mouseAtual
			projetil(&balaX, &balaY, &balaAtiva, principal.xi, principal.xf, principal.yi, principal.yf, largura_tela, &mouseAnterior, &principal.virou, &balaVirou, principal.ativo);

			//inimigos
			if (inimigo[0].vida > 0) {
				al_draw_filled_circle(inimigo[0].x, inimigo[0].y, inimigo[0].raio, al_map_rgb(0, 0, 255));
			}

			if (inimigo[1].vida > 0) {
				al_draw_filled_circle(inimigo[1].x, inimigo[1].y, inimigo[1].raio, al_map_rgb(255, 255, 255));
			}

			if (inimigo[2].vida > 0) {
				al_draw_filled_circle(inimigo[2].x, inimigo[2].y, inimigo[2].raio, al_map_rgb(255, 255, 0));
			}

			al_draw_filled_rectangle(0, 400, largura_tela, altura_tela, al_map_rgb(0, 0, 0));//chao

		}
		al_flip_display();
	}

	al_destroy_bitmap(menuImg);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_display(display);

	return 0;
}
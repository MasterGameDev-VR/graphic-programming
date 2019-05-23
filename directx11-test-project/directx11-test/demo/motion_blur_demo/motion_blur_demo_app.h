#pragma once
#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>
#include <render/renderable.h>
/*
Il motion blur è una tecnica di post - processing che si realizza su oggetti in movimento e permette a questi ultimi
di lasciare una scia semi - invisibile durante il loro movimento
Per implementarlo è necessario avere una texture2D usata come buffer, avente width e height uguali a quelli della
scena da renderizzare, e nella quale vengono immagazzinate le DIREZIONI DI MOVIMENTO DI CIASCUNO dei pixel che compongono un oggett
questa texture deve essere riempita con una passata di rendering
*/
/* dovremmo creare 
  - una classe  nel namespace xtest/demo
 che eredita da DirectxApp, MouseListener e Keyboard Listener
 - in questa classe inseriamo un oggetto di tipo RenderPass che chiamiamo MotionBlurPass
 - poi creiamo una classe MotionBlurMap usando la gerarchia suggerita dal professore
 - nella classe MotionBlurMap dovremmo inserire una Texture2D nella quale memorizziamo per ogni pixel
 la distanza tra la posizione in cui si trova nel frame corrente e quella in cui si trovava nel frame precedente
 - la passata di rendering deve inserire i dati in questa texture
 - serviranno una struct PerObjectMotionBlurData e un metodo ToPerObjectMotionBlurData: in questa struct ci mettiamo
  le matrici ottenute dai prodotti WVP relative a due frame (quello attuale e quello precedente)
- 

*/
namespace xtest {
	namespace demo {

		class motion_blur_demo_app : public application::DirectxApp, public input::KeyboardListener, public input::MouseListener
		{
		public:
			motion_blur_demo_app();
			virtual ~motion_blur_demo_app();
		};
	}
}

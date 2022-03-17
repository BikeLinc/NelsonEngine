/*****************************************************************//**
 * \file   Game.h
 * \brief  Abstracts boilerplate code required for running a game and
 * editor. 
 * 
 * Class will be abstracted to create a runnable game.
 * 
 * \author Lincoln Scheer
 * \date   March 2022
 *********************************************************************/

#pragma once

#include "Nelson.h"
namespace Nelson {
        class Game {
        public:
                /**
                 * Instantiate framework systems.
                 */
                Game();				

                /**
                 * Run game and game editor.
                 */
                void run();

        protected:
                /* Scene */
                Scene* scene = nullptr;

                /* Timing */
                float deltaTime = 0.0f;

                /**
                 * Implementable function that is called before the game loop.
                 *
                 */
                virtual void start() = 0;

                /**
                 * Implementable function that is called every frame..
                 *
                 */
                virtual void update() = 0;

                /**
                 * Implementable function that is called after the game loop..
                 *
                 */
                virtual void dispose() = 0;

                /**
                 * Send message through message bus to framework systems.
                 * 
                 * \param message
                 */
                void sendMessage(Message message);
                

        private:
                /* Framework Systems */
                MessageBus bus;
                Console* console = nullptr;
                Window* window = nullptr;
                Renderer* renderer = nullptr;
                Editor* editor = nullptr;

                /**
                 * Call initialize method for all systems.
                 */
                void _init();

                /**
                 * Call dispose method for all systems.
                 */
                void _dispose();

                /**
                 * Game Loop.
                 */
                void _loop();                   // Game Loop
        };
}

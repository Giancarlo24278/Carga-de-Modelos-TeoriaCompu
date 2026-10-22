#pragma once
#include <cstdint>
#include <map>

struct Color { uint8_t r, g, b; };
enum class Part { Body, LeftTurbine, RightTurbine, LowerCylinders,
                  UpperCylinders, Roof, Window };

// CAMBIA ESTOS RGB (valores de 0 a 255).
// Negro puro para el fondo de la ventana del programa:
inline constexpr Color BACKGROUND{0, 0, 0};
inline constexpr Color SALMON{250, 128, 114};
inline constexpr Color TEAL{64, 190, 200};
inline constexpr Color BLACK{0, 0, 0};

// false = colores RGB planos exactos. true = sombras para ver el volumen.
inline constexpr bool USE_LIGHTING = true;

inline Color colorForPart(Part part) {
    switch (part) {
        case Part::Body:           return SALMON; // Cuerpo de la nave.
        case Part::LeftTurbine:    return SALMON; // Turbina hacia Z negativo.
        case Part::RightTurbine:   return SALMON; // Turbina hacia Z positivo.
        case Part::LowerCylinders:return SALMON; // Conectores inferiores.
        case Part::UpperCylinders:return TEAL;   // Cilindros superiores.
        case Part::Roof:           return TEAL;   // Parte de arriba del cuerpo.
        case Part::Window:         return BLACK;  // Vidrio inclinado de la cabina.
    }
    return SALMON;
}

// OPCIONAL: pintar una cara especifica. ID = orden de la linea 'f', desde 1.
// Para activar el ejemplo, quita las dos barras de la linea:
inline const std::map<int, Color> FACE_COLORS = {
    // {2283, Color{255, 255, 0}}, // Esa cara se pintaria amarilla.
};

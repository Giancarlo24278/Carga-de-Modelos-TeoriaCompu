#pragma once
#include <cstdint>
#include <map>

struct Color { uint8_t r, g, b; };
enum class Part { Body, LeftTurbine, RightTurbine, TurbineRim, TurbineCore,
                  LowerCylinders, UpperCylinders, Nozzle, Roof, Window,
                  RidgeFront, RidgeMiddle, RidgeBack };

// CAMBIA ESTOS RGB (valores de 0 a 255).
// Colores tomados de la imagen de referencia de la nave.
// Negro puro para el fondo de la ventana del programa:
inline constexpr Color BACKGROUND{0, 0, 0};
inline constexpr Color ORANGE{240, 100, 68};     // Naranja rojizo del casco.
inline constexpr Color TEAL{50, 172, 150};       // Turquesa del borde de las turbinas.
inline constexpr Color CORE{62, 96, 118};        // Interior oscuro con brillo azul.
inline constexpr Color NOZZLE_GRAY{128, 128, 112}; // Gris de las toberas.
inline constexpr Color DARK_PURPLE{58, 38, 62};  // Vidrio oscuro de la cabina.

// false = colores RGB planos exactos. true = sombras para ver el volumen.
inline constexpr bool USE_LIGHTING = true;

inline Color colorForPart(Part part) {
    switch (part) {
        case Part::Body:           return ORANGE;      // Cuerpo de la nave.
        case Part::LeftTurbine:    return ORANGE;      // Dientes de la turbina Z negativo.
        case Part::RightTurbine:   return ORANGE;      // Dientes de la turbina Z positivo.
        case Part::TurbineRim:     return TEAL;        // Pared exterior de ambas turbinas.
        case Part::TurbineCore:    return CORE;        // Disco interior de ambas turbinas.
        case Part::LowerCylinders: return ORANGE;      // Conectores inferiores.
        case Part::UpperCylinders: return ORANGE;      // Capsulas de los motores.
        case Part::Nozzle:         return NOZZLE_GRAY; // Punta y tapa de los cilindros.
        case Part::Roof:           return ORANGE;      // Parte de arriba del cuerpo.
        case Part::Window:         return DARK_PURPLE; // Filas 1-2-3 y 4-5-6 del frente.
        case Part::RidgeFront:     return TEAL;        // Salidita de arriba: punta.
        case Part::RidgeMiddle:    return NOZZLE_GRAY; // Salidita de arriba: anillo gris.
        case Part::RidgeBack:      return ORANGE;      // Salidita de arriba: caja trasera.
    }
    return ORANGE;
}

// OPCIONAL: pintar una cara especifica. ID = orden de la linea 'f', desde 1.
// Para activar el ejemplo, quita las dos barras de la linea:
inline const std::map<int, Color> FACE_COLORS = {
    // {2283, Color{255, 255, 0}}, // Esa cara se pintaria amarilla.
};

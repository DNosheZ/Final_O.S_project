Paso 1: Configurar el entorno del programa
    Definir constantes:

        Espacio virtual de 32 bits con páginas de 4 KiB implica que cada página tiene 12 bits para el desplazamiento.
        Constantes para los tamaños de TLB y entradas.
    Estructura del programa:

    Implementa el código en C.
        Utiliza funciones para convertir de decimal a binario y de binario a decimal.
        Implementa la lógica para gestionar el TLB con memoria dinámica.
Paso 2: Implementar la función principal (main)
    Leer direcciones de memoria:

    Repetir hasta que el usuario ingrese la letra s.
    Obtener el tiempo de operación:

        Usa clock_gettime() antes y después de la traducción para medir el tiempo en segundos. (falta desde aqui)
    Calcular la página y el desplazamiento:

        Para una dirección decimal ingresada, calcula:
        Número de página en decimal y binario.
        Desplazamiento en decimal y binario.
    Gestión del TLB:

        Usa punteros para manejar el TLB sin estructuras.
        Aplica la política FIFO para la entrada y reemplazo en el TLB.
        Si hay un TLB Hit, muestra los detalles; si no, gestiona el TLB Miss y reemplazo según FIFO.
Paso 3: Implementar funciones auxiliares
    Función para convertir de decimal a binario:

    Usa operadores >> y & para construir el número binario bit a bit.
    Función para convertir de binario a decimal:

    Recorre el binario acumulando el resultado como potencias de 2.
    Función para gestionar el TLB:

    Implementa la lógica para insertar, buscar y reemplazar entradas en el TLB usando punteros.
Paso 4: Liberar recursos
    Liberar memoria del TLB:
        Asegúrate de liberar toda la memoria dinámica utilizada para evitar fugas.
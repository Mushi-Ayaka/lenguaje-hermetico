from experimento_silla import render_image


if __name__ == "__main__":
    output_file = render_image(width=512, height=512, filename="silla_estatica.png")
    print(f"Imagen guardada en: {output_file}")

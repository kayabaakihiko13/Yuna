#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "psnr_evaluator.hh"

int main(int argc, char** argv) {
    std::string path_asli = (argc > 1) ? argv[1] : "docs/images/65056cf873ff6c1ac9240af3bfe4d8c6.jpg";
    std::string path_enhance = (argc > 2) ? argv[2] : "docs/images/output_4x.jpg";

    Image img1, img2;
    if (!load_image(path_asli, img1)) {
        std::cerr << "Error: Gagal memuat " << path_asli << std::endl;
        return 1;
    }
    if (!load_image(path_enhance, img2)) {
        std::cerr << "Error: Gagal memuat " << path_enhance << std::endl;
        return 1;
    }

    std::cout << "==========================================" << std::endl;
    std::cout << "  HASIL EVALUASI IMAGE ENHANCEMENT" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Ukuran     : " << img1.width << " x " << img1.height << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    float psnr = calculate_psnr(img1, img2);

    std::cout << "Nilai PSNR : " << psnr << " dB" << std::endl;
    std::cout << "==========================================" << std::endl;

    return 0;
}
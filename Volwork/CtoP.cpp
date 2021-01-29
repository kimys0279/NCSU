#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/legacy/legacy.hpp"
#include "opencv/cxcore.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <stdio.h>

IplImage* marker_mask = 0;
IplImage* marker_mask2 = 0;
IplImage* mascara = 0;
IplImage* markers = 0, * markers_8bits = 0;
IplImage* img0 = 0, * img = 0, * img_gray = 0, * wshed = 0, * img_inicial = 0;
IplImage* L = 0, * U = 0, * X = 0, * V = 0, * imgluxv = 0;
IplImage* r_plane = 0, * g_plane = 0, * b_plane = 0;
IplImage* r_plane_marca = 0, * g_plane_marca = 0, * b_plane_marca = 0;
IplImage* imghsv = 0, * imgray = 0, * imgxyz = 0;
IplImage* imglab = 0, * imgluv = 0, * imgycc = 0;
IplImage* imgseg = 0, * imgmks = 0, * imgmks_copia = 0, * imgmks1 = 0, * imgmks2 = 0, * imgmks1_temp = 0, * imgseg_3canales = 0;
IplImage* r_plane_seg = 0, * g_plane_seg = 0, * b_plane_seg = 0, * zonas = 0, * zonas_gral = 0, * zonas_ori = 0;

CvPoint prev_pt = { -1,-1 };
CvPoint pt2 = { -1,-1 };

int x_ini = 0;
int y_ini = 0;
int x_fin = 0;
int y_fin = 0;

CvScalar r;
CvScalar g;
CvScalar b;
double l;
double u;
double x;
double v;
int M = 256;

char Buffer1[30];
char Buffer[34];
char Buffer2[34];
char str1[100];
char str2[100];
char str3[100];
char str4[100];
char str5[100];
char str6[100];

int flag_carga = 0;

bool is_selecting = false;

void on_mouse(int event, int x, int y, int flags, void* param)
{
    if (!img)
        return;

    if (flags & CV_EVENT_FLAG_MBUTTON /*& CV_EVENT_FLAG_CTRLKEY*/) {

        if (event == CV_EVENT_MBUTTONUP || !(flags & CV_EVENT_FLAG_MBUTTON))
            prev_pt = cvPoint(-1, -1);
        else if (event == CV_EVENT_MBUTTONDOWN)
            prev_pt = cvPoint(x, y);
        else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_MBUTTON))
        {
            CvPoint pt = cvPoint(x, y);
            if (prev_pt.x < 0)
                prev_pt = pt;
            cvLine(marker_mask, prev_pt, pt, cvScalarAll(0), 10, 10, 0);
            cvLine(img, prev_pt, pt, cvScalarAll(0), 10, 10, 0);
            prev_pt = pt;
            cvShowImage("image", img);
            cvSaveImage("image_selec_WS.png", img);

            sprintf(str4, "marcas_%s", Buffer);
            cvSaveImage("marcas.jpg", marker_mask);
            cvSaveImage(str4, marker_mask);
        }

    }

    else if (flags & CV_EVENT_FLAG_LBUTTON) {

        if (event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON))
            prev_pt = cvPoint(-1, -1);
        else if (event == CV_EVENT_LBUTTONDOWN)
            prev_pt = cvPoint(x, y);
        else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
        {
            CvPoint pt = cvPoint(x, y);
            if (prev_pt.x < 0)
                prev_pt = pt;
            cvLine(marker_mask, prev_pt, pt, cvScalarAll(255), 7, 3, 0);
            cvLine(img, prev_pt, pt, cvScalarAll(255), 5, 3, 0);
            prev_pt = pt;
            cvShowImage("image", img);
            cvSaveImage("image_selec_WS.png", img);

            sprintf(str4, "marcas_%s", Buffer);
            cvSaveImage("marcas.jpg", marker_mask);
            cvSaveImage(str4, marker_mask);
        }

    }
}

void mouse_func_mark(int event, int x, int y, int flags, void* param)
{
    if (!img)
        return;

    //    prev_pt = cvPoint(-1,-1);
    //    pt2 = cvPoint(-1,-1);

    if (flags & CV_EVENT_FLAG_LBUTTON) {

        switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            prev_pt = cvPoint(x, y);
            pt2 = cvPoint(x, y);
            is_selecting = true;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (is_selecting)
                pt2 = cvPoint(x, y);
            break;
        case CV_EVENT_LBUTTONUP:
            pt2 = cvPoint(x, y);
            is_selecting = false;
            break;
        }

        printf("\npunto inicial x: %d, y:%d\npunto final x:%d, y: %d\n", prev_pt.x, prev_pt.y, pt2.x, pt2.y);

    }



    return;

}

void ON_LUX(int a)
{
    L = cvCreateImage(cvGetSize(img0), 8, 1);
    U = cvCreateImage(cvGetSize(img0), 8, 1);
    X = cvCreateImage(cvGetSize(img0), 8, 1);
    V = cvCreateImage(cvGetSize(img0), 8, 1);

    int rows = img0->height;
    int columns = img0->width;
    //printf("\nImagen de tamaÃ±o %d x %d",rows, columns);

    //r = cvGet2D(r_plane,286,351);
    //printf("\nValor R %g", r.val[0]);

    for (int i = 0; i < rows; i++)
    {
        //printf("\nel valor de i es: %d", i);

        for (int j = 0; j < columns; j++)
        {
            //printf("\nel valor de j es: %d", j);
            r = cvGet2D(r_plane, i, j);
            //printf("\nValor R %g", r.val[0]);
            g = cvGet2D(g_plane, i, j);
            b = cvGet2D(b_plane, i, j);
            l = (pow((r.val[0] + 1), 0.3)) * (pow((g.val[0] + 1), 0.6)) * (pow((b.val[0] + 1), 0.1));
            if (r.val[0] < l)
            {
                u = (M / 2) * ((r.val[0] + 1) / (l + 1));
                x = (M / 2) * ((b.val[0] + 1) / (l + 1));
                v = (M / 2) * ((g.val[0] + 1) / (l + 1));
            }
            else
            {
                u = M - ((M / 2) * ((l + 1) / (r.val[0] + 1)));
                x = M - ((M / 2) * ((l + 1) / (b.val[0] + 1)));
                v = M - ((M / 2) * ((l + 1) / (g.val[0] + 1)));
            }

            cvSet2D(L, i, j, cvScalar(l));
            cvSet2D(U, i, j, cvScalar(u));
            cvSet2D(X, i, j, cvScalar(x));
            cvSet2D(V, i, j, cvScalar(v));

        }
    }
}


int main(int argc, char** argv)
{


    FILE* Process;
    FILE* Contornos;

    int rama = 0;

    printf("Ingrese nÃºmero de rama\n");
    scanf("%d", &rama);

    Process = fopen("vectores.txt", "a+t");
    fprintf(Process, "%s", "Rama\t Archivo\t Clase\t m_r\t m_g\t m_b\t m_h\t m_s\t m_v\t m_y\t m_cr\t m_cb\t m_l\t m_u\t m_v\t m_x\t m_yy\t m_z\t m_ll\t m_a\t m_bb\t m_uu\t m_xx\t m_vvv\t s_r\t s_g\t s_b\t s_h\t s_s\t s_v\t s_y\t s_cr\t s_cb\t s_l\t s_u\t s_v\t s_x\t s_yy\t s_z\t s_ll\t s_a\t s_bb\t s_uu\t s_xx\t s_vvv\t area\t perimetro\t centroX\t centroY\n ");
    fclose(Process);

    char* filename;

    FILE* lista = fopen("lista.txt", "r");
    fseek(lista, 0, SEEK_END);
    int numeroRegistros = ftell(lista) / sizeof(Buffer1);
    printf("Numero de archivos - %d\n", numeroRegistros);
    fclose(lista);
    int N = 0;

    for (N; N < numeroRegistros; N++)
    {
        printf("Numero de iteracion - %d\n", N);
        lista = fopen("lista.txt", "r");
        fseek(lista, N * sizeof(Buffer1), SEEK_SET);
        fscanf(lista, "%s", Buffer1);

        printf("El nombre del archivo es - %s\n", Buffer1);
        fclose(lista);

        Buffer[0] = Buffer1[0];
        Buffer[1] = Buffer1[1];
        Buffer[2] = Buffer1[2];
        Buffer[3] = Buffer1[3];
        Buffer[4] = Buffer1[4];
        Buffer[5] = Buffer1[5];
        Buffer[6] = Buffer1[6];
        Buffer[7] = Buffer1[7];
        Buffer[8] = Buffer1[8];
        Buffer[9] = Buffer1[9];
        Buffer[10] = Buffer1[10];
        Buffer[11] = Buffer1[11];
        Buffer[12] = Buffer1[12];
        Buffer[13] = Buffer1[13];
        Buffer[14] = Buffer1[14];
        Buffer[15] = Buffer1[15];
        Buffer[16] = Buffer1[16];
        Buffer[17] = Buffer1[17];
        Buffer[18] = Buffer1[18];
        Buffer[19] = Buffer1[19];
        Buffer[20] = Buffer1[20];
        Buffer[21] = Buffer1[21];
        Buffer[22] = Buffer1[22];
        Buffer[23] = Buffer1[23];
        Buffer[24] = Buffer1[24];
        Buffer[25] = Buffer1[25];
        Buffer[26] = Buffer1[26];
        Buffer[27] = Buffer1[27];
        Buffer[28] = Buffer1[28];
        Buffer[29] = Buffer1[29];



        //help();

    //    filename = argc == 2 ? argv[1] : (char*)"i1058.jpg";
        filename = argc == 2 ? argv[1] : (char*)strcat(Buffer, ".png");

        printf("El nombre del archivo es - %s\n", Buffer);

        //char* filename = argc >= 2 ? argv[1] : (char*)"color.jpg";

        CvMemStorage* storage = cvCreateMemStorage(0);
        CvRNG rng = cvRNG(-1);

        if ((img_inicial = cvLoadImage(filename, 1)) == 0)
            return 0;

        printf("Opciones clave: \n"
            "\tESC - Salir del programa\n"
            "\tr - Volver a la imagen original\n"
            "\tw or SPACE - Correr el algoritmo crecimiento de regiones\n"
            "\to - Etiquetar cada una de las zonas marcadas\n"
            "\tm - Cargar conjunto de marcas anteriores\n"
            "\t\t(Antes de ejecutarlo verificar marcas en las zonas deseadas)\n");

        int height = img_inicial->height;
        int width = img_inicial->width;
        img0 = cvCreateImage(cvSize(int(width * 0.5), int(height * 0.5)), IPL_DEPTH_8U, 3);

        cvResize(img_inicial, img0, CV_INTER_AREA);

        cvNamedWindow("image", 0);
        cvNamedWindow("watershed transform", 0);

        IplImage* src = cvCreateImage(cvGetSize(img0), IPL_DEPTH_8U, 3);
        IplImage* dst = cvCreateImage(cvGetSize(img0), IPL_DEPTH_8U, 3);

        src = cvCloneImage(img0);
        img = cvCloneImage(img0);
        img_gray = cvCloneImage(img0);
        wshed = cvCloneImage(img0);
        marker_mask = cvCreateImage(cvGetSize(img), 8, 1);
        marker_mask2 = cvCreateImage(cvGetSize(img), 8, 1);
        markers = cvCreateImage(cvGetSize(img), IPL_DEPTH_32S, 1);
        markers_8bits = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
        cvCvtColor(img, marker_mask, CV_BGR2GRAY);
        cvCvtColor(marker_mask, img_gray, CV_GRAY2BGR);

        cvCvtColor(img, marker_mask2, CV_BGR2GRAY);
        cvCvtColor(marker_mask2, img_gray, CV_GRAY2BGR);

        dst = cvCloneImage(src);

        //conversiones a color

        r_plane = cvCreateImage(cvGetSize(img0), 8, 1);
        g_plane = cvCreateImage(cvGetSize(img0), 8, 1);
        b_plane = cvCreateImage(cvGetSize(img0), 8, 1);

        r_plane_marca = cvCreateImage(cvGetSize(img0), 8, 1);
        g_plane_marca = cvCreateImage(cvGetSize(img0), 8, 1);
        b_plane_marca = cvCreateImage(cvGetSize(img0), 8, 1);

        cvSplit(img0, b_plane, g_plane, r_plane, NULL);

        ON_LUX(1);

        imgluxv = cvCreateImage(cvGetSize(img0), 8, 3);
        imghsv = cvCreateImage(cvGetSize(img0), 8, 3);
        imgray = cvCreateImage(cvGetSize(img0), 8, 1);
        imgxyz = cvCreateImage(cvGetSize(img0), 8, 3);
        imglab = cvCreateImage(cvGetSize(img0), 8, 3);
        imgluv = cvCreateImage(cvGetSize(img0), 8, 3);
        imgycc = cvCreateImage(cvGetSize(img0), 8, 3);

        imgseg = cvCreateImage(cvGetSize(img0), 8, 1);
        imgmks = cvCreateImage(cvGetSize(img0), 8, 1);
        imgmks_copia = cvCreateImage(cvGetSize(img0), 8, 1);
        imgmks1 = cvCreateImage(cvGetSize(img0), 8, 3);
        imgmks2 = cvCreateImage(cvGetSize(img0), 8, 1);
        imgmks1_temp = cvCreateImage(cvGetSize(img0), 8, 3);
        imgseg_3canales = cvCreateImage(cvGetSize(img0), 8, 3);

        r_plane_seg = cvCreateImage(cvGetSize(img0), 8, 1);
        g_plane_seg = cvCreateImage(cvGetSize(img0), 8, 1);
        b_plane_seg = cvCreateImage(cvGetSize(img0), 8, 1);

        zonas = cvCreateImage(cvGetSize(img0), 8, 3);
        zonas_gral = cvCreateImage(cvGetSize(img0), 8, 3);
        zonas_ori = cvCreateImage(cvGetSize(img0), 8, 3);

        cvMerge(U, X, V, NULL, imgluxv);
        cvCvtColor(img0, imghsv, CV_BGR2HSV);
        cvCvtColor(img0, imgray, CV_BGR2GRAY);
        cvCvtColor(img0, imgxyz, CV_BGR2XYZ);
        cvCvtColor(img0, imglab, CV_BGR2Lab);
        cvCvtColor(img0, imgluv, CV_BGR2Luv);
        cvCvtColor(img0, imgycc, CV_BGR2YCrCb);

    loop2:

        if (flag_carga == 0)
            cvZero(marker_mask);
        else
            cvCopy(mascara, marker_mask);

        cvZero(marker_mask2);
        cvZero(wshed);

        cvShowImage("image", img);
        cvShowImage("watershed transform", wshed);
        cvSetMouseCallback("image", on_mouse, 0);

        for (;;)
        {
            int c = cvWaitKey(0);
            int contador1 = 0;

            if ((char)c == 27)
                break;

            if ((char)c == 'f')
            {
                //            strcpy(Buffer2,Buffer);
                //            printf( "archivo anterior %s\n", Buffer2 );
                flag_carga = 0;
                goto loop;

            }

            if ((char)c == 'm')
            {
            loop3:
                cvZero(marker_mask);
                cvZero(mascara);
                sprintf(str6, "marcas_%s", Buffer2);
                printf("Archivo de marcas %s\n", str6);

                mascara = cvLoadImage(str6, 0);
                int canales = mascara->nChannels;
                int tipo = mascara->depth;
                printf("Canales\t%d y Tipo\t%d\n", canales, tipo);
                cvCopy(mascara, marker_mask);

                flag_carga = 1;

                cvOr(marker_mask, b_plane, b_plane_marca);
                cvOr(marker_mask, g_plane, g_plane_marca);
                cvOr(marker_mask, r_plane, r_plane_marca);

                cvMerge(b_plane_marca, g_plane_marca, r_plane_marca, NULL, img);

                cvShowImage("image", img);
            loop6:
                cvSetMouseCallback("image", mouse_func_mark, 0);

                cvWaitKey(0);

                for (;;)
                {
                    int e = cvWaitKey(0);

                    int x_shif = 0;
                    //x_shif = int(prev_pt.x) - int(pt2.x);
                    x_shif = int(pt2.x) - int(prev_pt.x);
                    //x_shif = x_fin - x_ini;
                    int y_shif = 0;
                    //y_shif = int(prev_pt.y) - int(pt2.y);
                    y_shif = int(pt2.y) - int(prev_pt.y);
                    //y_shif = y_fin - y_ini;

                    printf("\nCorrimiento en X: %d, Corrimiento en Y: %d\n", x_shif, y_shif);
                    printf("\nInicial X: %d, Inicial Y: %d\n", prev_pt.x, prev_pt.y);
                    printf("\nFinal X: %d, Final Y: %d\n", pt2.x, pt2.y);

                    cvZero(marker_mask);


                    if (x_shif < 0 & y_shif < 0)
                    {
                        cvSetImageROI(mascara, cvRect(int(abs(x_shif)), int(abs(y_shif)), mascara->width - 0, mascara->height - 0));
                        cvSetImageROI(marker_mask, cvRect(0, 0, mascara->width - int(abs(x_shif)), mascara->height - int(abs(y_shif))));
                        cvCopy(mascara, marker_mask);
                        cvResetImageROI(mascara);
                        cvResetImageROI(marker_mask);
                        cvCopy(marker_mask, mascara);
                    }

                    else if (x_shif >= 0 & y_shif >= 0) {

                        cvSetImageROI(mascara, cvRect(0, 0, mascara->width - x_shif, mascara->height - y_shif));
                        cvSetImageROI(marker_mask, cvRect(x_shif, y_shif, mascara->width - 0, mascara->height - 0));
                        cvCopy(mascara, marker_mask);
                        cvResetImageROI(mascara);
                        cvResetImageROI(marker_mask);
                        cvCopy(marker_mask, mascara);
                    }

                    else if (x_shif >= 0 & y_shif < 0) {

                        cvSetImageROI(mascara, cvRect(0, int(abs(y_shif)), mascara->width - x_shif, mascara->height - 0));
                        cvSetImageROI(marker_mask, cvRect(x_shif, 0, mascara->width - 0, mascara->height - int(abs(y_shif))));
                        cvCopy(mascara, marker_mask);
                        cvResetImageROI(mascara);
                        cvResetImageROI(marker_mask);
                        cvCopy(marker_mask, mascara);
                    }

                    else if (x_shif < 0 & y_shif >= 0) {

                        cvSetImageROI(mascara, cvRect(int(abs(x_shif)), 0, mascara->width - 0, mascara->height - y_shif));
                        cvSetImageROI(marker_mask, cvRect(0, y_shif, mascara->width - int(abs(x_shif)), mascara->height - 0));
                        cvCopy(mascara, marker_mask);
                        cvResetImageROI(mascara);
                        cvResetImageROI(marker_mask);
                        cvCopy(marker_mask, mascara);
                    }





                    cvOr(marker_mask, b_plane, b_plane_marca);
                    cvOr(marker_mask, g_plane, g_plane_marca);
                    cvOr(marker_mask, r_plane, r_plane_marca);

                    cvMerge(b_plane_marca, g_plane_marca, r_plane_marca, NULL, img);

                    cvShowImage("image", img);

                    cvWaitKey();

                    printf("\nFavor remarcar o borrar si es necesario\n");

                    goto loop2;
                    //                    cvSetMouseCallback( "image", on_mouse, 0 );

                    printf("\nLuego de definir el corrimiento de la mascara\nfavor seleccionar una de las siguientes opciones:\n"
                        "\tESC - Salir del programa\n"
                        "\tr - Volver a la imagen original\n"
                        "\tw or SPACE - Correr el algoritmo crecimiento de regiones\n"
                        "\to - Etiquetar cada una de las zonas marcadas\n"
                        "\tm - Cargar conjunto de marcas anteriores\n"
                        "\t\t(Antes de ejecutarlo verificar marcas en las zonas deseadas)\n");
                    //                    if( (char)e == 'w' || (char)e == ' ' )
                    //                        goto loop4;

                    //                    if( (char)e == 'r' )
                    //                        goto loop5;

                    ////                    if( (char)e == 27 )
                    ////                        goto loop2;

                    //                    if( (char)e == 'e' ){
                    //                        goto loop3;
                    //                    }



                }



            }


            if ((char)c == 'r')
            {
            loop5:
                cvZero(marker_mask);
                cvZero(marker_mask2);
                cvCopy(img0, img);
                cvShowImage("image", img);
            }

            if ((char)c == 'w' || (char)c == ' ')
            {
            loop4:

                CvSeq* contours = 0;
                CvMat* color_tab = 0;
                int i, j, comp_count = 0;

                cvClearMemStorage(storage);
                cvZero(wshed);


                cvCopy(marker_mask, marker_mask2);
                //marker_mask = cvLoadImage( "wshed_mask.png", 0 );
                cvFindContours(marker_mask2, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
                //CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
                cvZero(markers);
                cvZero(markers_8bits);

                CvSeq* contours_poly2;
                CvMemStorage* storage_poly2;
                storage_poly2 = cvCreateMemStorage(0);

                for (; contours != 0; contours = contours->h_next, comp_count++)
                {
                    contours_poly2 = cvApproxPoly(contours, sizeof(CvContour), storage_poly2, CV_POLY_APPROX_DP, 2, 0);
                    cvDrawContours(markers, contours_poly2, cvScalarAll(comp_count + 1),
                        cvScalarAll(comp_count + 1), -1, -1, 8, cvPoint(0, 0));
                }

                if (comp_count == 0)
                    continue;

                color_tab = cvCreateMat(1, comp_count, CV_8UC3);
                for (i = 0; i < comp_count; i++)
                {
                    uchar* ptr = color_tab->data.ptr + i * 3;
                    ptr[0] = (uchar)(cvRandInt(&rng) % 180 + 50);
                    ptr[1] = (uchar)(cvRandInt(&rng) % 180 + 50);
                    ptr[2] = (uchar)(cvRandInt(&rng) % 180 + 50);
                }

                cvWaitKey(1);

                {
                    double t = (double)cvGetTickCount();
                    //cvWatershed( img0, markers );
                    cvWatershed(dst, markers);
                    t = (double)cvGetTickCount() - t;
                    printf("exec time = %gms\n", t / (cvGetTickFrequency() * 1000.));
                    printf("numero se zonas seleccionadas: \t%d\n", comp_count);

                }

                // paint the watershed image
                for (i = 0; i < markers->height; i++)
                    for (j = 0; j < markers->width; j++)
                    {
                        int idx = CV_IMAGE_ELEM(markers, int, i, j);
                        uchar* dst = &CV_IMAGE_ELEM(wshed, uchar, i, j * 3);
                        if (idx == -1)
                            dst[0] = dst[1] = dst[2] = (uchar)255;
                        else if (idx <= 0 || idx > comp_count)
                            dst[0] = dst[1] = dst[2] = (uchar)0; // should not get here
                        else
                        {
                            uchar* ptr = color_tab->data.ptr + (idx - 1) * 3;
                            dst[0] = ptr[0]; dst[1] = ptr[1]; dst[2] = ptr[2];
                        }
                    }

                cvAddWeighted(wshed, 0.5, img_gray, 0.5, 0, wshed);
                cvShowImage("watershed transform", wshed);
                cvSaveImage("WS_transform.png", wshed);
                cvReleaseMat(&color_tab);

                int k = cvWaitKey(0);
                int b = 0;


                if ((char)k == 'o' || (char)k == 'O')
                {

                    cvZero(zonas);
                    cvZero(zonas_gral);
                    cvZero(zonas_ori);
                    cvZero(imgmks);
                    cvZero(imgmks_copia);

                    cvZero(b_plane_seg);
                    cvZero(g_plane_seg);
                    cvZero(r_plane_seg);

                    cvZero(imgseg);

                    printf("PUNTO PARA PROCESO DE SECUENCIAS POR SEGMENTO\n");


                    CvScalar pixel;
                    double minVal = 0, maxVal = 0;
                    cvMinMaxLoc(markers, &minVal, &maxVal);
                    printf("Los valores mÃ¡ximo y mÃ­nimo son: \t%g\t%g\n", maxVal, minVal);

                    cvConvertScale(markers, markers_8bits);
                    //cvShowImage("markers_8bits", markers_8bits);

                    CvMemStorage* storage2 = cvCreateMemStorage(0);
                    cvClearMemStorage(storage2);
                    CvSeq* contours2 = 0;

                    CvMoments moments_finales;
                    double M00_final, M01_final, M10_final;
                    cvZero(zonas);
                    cvZero(zonas_gral);
                    cvZero(zonas_ori);

                    for (int zona = 1; zona < comp_count + 1; zona++)
                    {
                        printf("numero de zona analizada: \t%d\n", zona);
                        // SEGMENTAR ZONA DE MARCA

                        for (i = 0; i < markers_8bits->height; i++)
                            for (j = 0; j < markers_8bits->width; j++)
                            {
                                pixel = cvGet2D(markers_8bits, i, j);

                                if (pixel.val[0] == zona)
                                {
                                    cvSet2D(imgmks_copia, i, j, cvScalar(255));
                                }
                                else
                                    cvSet2D(imgmks_copia, i, j, cvScalar(0));
                            }

                        //cvShowImage("Umbral_marcas", imgmks);


                        cvCopy(imgmks_copia, imgmks);


                        cvFindContours(imgmks, storage2, &contours2, sizeof(CvContour), CV_RETR_LIST);//CV_RETR_EXTERNAL);//, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
                                    //CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

                        //cvDrawContours(imgseg, contours2, cvScalar(0,0,255),cvScalar(255,0,0),0,2,8);
                        cvDrawContours(imgseg, contours2, cvScalar(255), cvScalar(0), 0, CV_FILLED);
                        //cvShowImage("segmented", imgseg);

                        cvMoments(contours2, &moments_finales);
                        M00_final = cvGetSpatialMoment(&moments_finales, 0, 0);
                        M10_final = cvGetSpatialMoment(&moments_finales, 1, 0);
                        M01_final = cvGetSpatialMoment(&moments_finales, 0, 1);
                        int centroX = (int)(M10_final / M00_final);
                        int centroY = (int)(M01_final / M00_final);

                        double area_tod = fabs(cvContourArea(contours2, CV_WHOLE_SEQ, 0));

                        double perimeter_tod = cvContourPerimeter(contours2);

                        //double redondez_tod = 4*3.1416*area_tod/(pow(perimeter_tod,2));

                        cvAnd(imgseg, r_plane, r_plane_seg);
                        cvAnd(imgseg, g_plane, g_plane_seg);
                        cvAnd(imgseg, b_plane, b_plane_seg);

                        cvMerge(imgseg, imgseg, imgseg, NULL, imgseg_3canales);

                        cvMerge(b_plane_seg, g_plane_seg, r_plane_seg, NULL, imgmks1);

                        cvNamedWindow("Zonas Seg", 0);
                        cvShowImage("Zonas Seg", imgmks1);

                        cvWaitKey(0);

                        int rows_contorno = imgseg->height;
                        int columns_contorno = imgseg->width;
                        //printf("%d, %ds:\n", rows_contorno, columns_contorno );

                        //CvMat* FEATURE_VEC = cvCreateMat( 14, 3, CV_32FC2);
                        CvScalar mean_rgb;
                        CvScalar std_rgb;
                        CvScalar mean_hsv;
                        CvScalar std_hsv;
                        CvScalar mean_ycrcb;
                        CvScalar std_ycrcb;
                        CvScalar mean_luv;
                        CvScalar std_luv;
                        CvScalar mean_lab;
                        CvScalar std_lab;
                        CvScalar mean_xyz;
                        CvScalar std_xyz;
                        CvScalar mean_luxv;
                        CvScalar std_luxv;

                        /*DeclaraciÃ³n de una sequencia*/
                        CvMemStorage* almacenamiento_de_frutos_RGB = cvCreateMemStorage(0);
                        CvSeq* rgb = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_RGB /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_HSV = cvCreateMemStorage(0);
                        CvSeq* hsv = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_HSV /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_LUV = cvCreateMemStorage(0);
                        CvSeq* luv = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_LUV /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_YCrCb = cvCreateMemStorage(0);
                        CvSeq* ycrcb = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_YCrCb /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_LAB = cvCreateMemStorage(0);
                        CvSeq* lab = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_LAB /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_XYZ = cvCreateMemStorage(0);
                        CvSeq* xyz = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_XYZ /* EL CvMemStorage DE ALMACENAMIENTO */);

                        CvMemStorage* almacenamiento_de_frutos_LUXV = cvCreateMemStorage(0);
                        CvSeq* luxv = cvCreateSeq(0, /* HAGA UNA SECUENCIA GENERICA */
                            sizeof(CvSeq), /* SIEMPRE VA ACÃ ??? */
                            sizeof(CvScalar), /* TAMAÃ‘O DE LA SECUENCIA */
                            almacenamiento_de_frutos_LUXV /* EL CvMemStorage DE ALMACENAMIENTO */);



                        for (int rr = 0; rr < rows_contorno; rr++)
                        {
                            for (int cc = 0; cc < columns_contorno; cc++)
                            {
                                CvScalar pixel = cvGet2D(imgseg, rr, cc);
                                if (pixel.val[0] != 0)
                                {

                                    //RGB
                                    CvScalar Prgb = cvGet2D(img, rr, cc);
                                    CvPoint3D32f p_rgb;
                                    p_rgb.x = (float(Prgb.val[2]));
                                    p_rgb.y = (float(Prgb.val[1]));
                                    p_rgb.z = (float(Prgb.val[0]));
                                    cvSeqPush(rgb, &p_rgb);

                                    //HSV
                                    CvScalar Phsv = cvGet2D(imghsv, rr, cc);
                                    CvPoint3D32f p_hsv;
                                    p_hsv.x = (float(Phsv.val[0]));
                                    p_hsv.y = (float(Phsv.val[1]));
                                    p_hsv.z = (float(Phsv.val[2]));
                                    cvSeqPush(hsv, &p_hsv);

                                    //YCrCb
                                    CvScalar Pycrcb = cvGet2D(imgycc, rr, cc);
                                    CvPoint3D32f p_ycrcb;
                                    p_ycrcb.x = (float(Pycrcb.val[0]));
                                    p_ycrcb.y = (float(Pycrcb.val[1]));
                                    p_ycrcb.z = (float(Pycrcb.val[2]));
                                    cvSeqPush(ycrcb, &p_ycrcb);

                                    //LUV
                                    CvScalar Pluv = cvGet2D(imgluv, rr, cc);
                                    CvPoint3D32f p_luv;
                                    p_luv.x = (float(Pluv.val[0]));
                                    p_luv.y = (float(Pluv.val[1]));
                                    p_luv.z = (float(Pluv.val[2]));
                                    cvSeqPush(luv, &p_luv);

                                    //Lab
                                    CvScalar Plab = cvGet2D(imglab, rr, cc);
                                    CvPoint3D32f p_lab;
                                    p_lab.x = (float(Plab.val[0]));
                                    p_lab.y = (float(Plab.val[1]));
                                    p_lab.z = (float(Plab.val[2]));
                                    cvSeqPush(lab, &p_lab);

                                    //XYZ
                                    CvScalar Pxyz = cvGet2D(imgxyz, rr, cc);
                                    CvPoint3D32f p_xyz;
                                    p_xyz.x = (float(Pxyz.val[0]));
                                    p_xyz.y = (float(Pxyz.val[1]));
                                    p_xyz.z = (float(Pxyz.val[2]));
                                    cvSeqPush(xyz, &p_xyz);

                                    //LUXV
                                    CvScalar Pluxv = cvGet2D(imgluxv, rr, cc);
                                    CvPoint3D32f p_luxv;
                                    p_luxv.x = (float(Pluxv.val[0]));
                                    p_luxv.y = (float(Pluxv.val[1]));
                                    p_luxv.z = (float(Pluxv.val[2]));
                                    cvSeqPush(luxv, &p_luxv);

                                }
                            }
                        }

                        CvMat* Mrgb = cvCreateMat(rgb->total, 1, CV_32FC3);
                        CvMat* Mr = cvCreateMat(rgb->total, 1, CV_32FC1);
                        CvMat* Mg = cvCreateMat(rgb->total, 1, CV_32FC1);
                        CvMat* Mb = cvCreateMat(rgb->total, 1, CV_32FC1);
                        CvMat* Mhsv = cvCreateMat(hsv->total, 1, CV_32FC3);
                        CvMat* Mh = cvCreateMat(hsv->total, 1, CV_32FC1);
                        CvMat* Ms = cvCreateMat(hsv->total, 1, CV_32FC1);
                        CvMat* Mv = cvCreateMat(hsv->total, 1, CV_32FC1);
                        CvMat* Mycrcb = cvCreateMat(ycrcb->total, 1, CV_32FC3);
                        CvMat* My = cvCreateMat(ycrcb->total, 1, CV_32FC1);
                        CvMat* Mcr = cvCreateMat(ycrcb->total, 1, CV_32FC1);
                        CvMat* Mcb = cvCreateMat(ycrcb->total, 1, CV_32FC1);
                        CvMat* Mluv = cvCreateMat(luv->total, 1, CV_32FC3);
                        CvMat* Ml = cvCreateMat(luv->total, 1, CV_32FC1);
                        CvMat* Mu = cvCreateMat(luv->total, 1, CV_32FC1);
                        CvMat* Mvv = cvCreateMat(luv->total, 1, CV_32FC1);
                        CvMat* Mlab = cvCreateMat(lab->total, 1, CV_32FC3);
                        CvMat* Mll = cvCreateMat(lab->total, 1, CV_32FC1);
                        CvMat* Ma = cvCreateMat(lab->total, 1, CV_32FC1);
                        CvMat* Mbb = cvCreateMat(lab->total, 1, CV_32FC1);
                        CvMat* Mxyz = cvCreateMat(xyz->total, 1, CV_32FC3);
                        CvMat* Mx = cvCreateMat(xyz->total, 1, CV_32FC1);
                        CvMat* Myy = cvCreateMat(xyz->total, 1, CV_32FC1);
                        CvMat* Mz = cvCreateMat(xyz->total, 1, CV_32FC1);
                        CvMat* Mluxv = cvCreateMat(luxv->total, 1, CV_32FC3);
                        CvMat* Muu = cvCreateMat(luxv->total, 1, CV_32FC1);
                        CvMat* Mxx = cvCreateMat(luxv->total, 1, CV_32FC1);
                        CvMat* Mvvv = cvCreateMat(luxv->total, 1, CV_32FC1);

                        cvZero(Mrgb);
                        cvZero(Mr);
                        cvZero(Mg);
                        cvZero(Mb);
                        cvZero(Mhsv);
                        cvZero(Mh);
                        cvZero(Ms);
                        cvZero(Mv);
                        cvZero(Mycrcb);
                        cvZero(My);
                        cvZero(Mcr);
                        cvZero(Mcb);
                        cvZero(Mluv);
                        cvZero(Ml);
                        cvZero(Mu);
                        cvZero(Mvv);
                        cvZero(Mlab);
                        cvZero(Mll);
                        cvZero(Ma);
                        cvZero(Mbb);
                        cvZero(Mxyz);
                        cvZero(Mx);
                        cvZero(Myy);
                        cvZero(Mz);
                        cvZero(Mluxv);
                        cvZero(Muu);
                        cvZero(Mxx);
                        cvZero(Mvvv);

                        for (int pp = 0; pp < rgb->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(rgb, pp);
                            CV_MAT_ELEM(*Mr, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Mg, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mb, float, pp, 0) = pt->z;

                        }

                        for (int pp = 0; pp < hsv->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(hsv, pp);
                            CV_MAT_ELEM(*Mh, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Ms, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mv, float, pp, 0) = pt->z;
                        }

                        for (int pp = 0; pp < ycrcb->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(ycrcb, pp);
                            CV_MAT_ELEM(*My, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Mcr, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mcb, float, pp, 0) = pt->z;
                        }

                        for (int pp = 0; pp < luv->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(luv, pp);
                            CV_MAT_ELEM(*Ml, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Mu, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mvv, float, pp, 0) = pt->z;
                        }

                        for (int pp = 0; pp < lab->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(lab, pp);
                            CV_MAT_ELEM(*Mll, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Ma, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mbb, float, pp, 0) = pt->z;
                        }

                        for (int pp = 0; pp < xyz->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(xyz, pp);
                            CV_MAT_ELEM(*Mx, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Myy, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mz, float, pp, 0) = pt->z;
                        }

                        for (int pp = 0; pp < luxv->total; pp++)
                        {
                            CvPoint3D32f* pt = (CvPoint3D32f*)cvGetSeqElem(luxv, pp);
                            CV_MAT_ELEM(*Muu, float, pp, 0) = pt->x;
                            CV_MAT_ELEM(*Mxx, float, pp, 0) = pt->y;
                            CV_MAT_ELEM(*Mvvv, float, pp, 0) = pt->z;
                        }


                        cvMerge(Mr, Mg, Mb, NULL, Mrgb);
                        cvAvgSdv(Mrgb, &mean_rgb, &std_rgb, NULL);
                        cvMerge(Mh, Ms, Mv, NULL, Mhsv);
                        cvAvgSdv(Mhsv, &mean_hsv, &std_hsv, NULL);
                        cvMerge(My, Mcr, Mcb, NULL, Mycrcb);
                        cvAvgSdv(Mycrcb, &mean_ycrcb, &std_ycrcb, NULL);
                        cvMerge(Ml, Mu, Mvv, NULL, Mluv);
                        cvAvgSdv(Mluv, &mean_luv, &std_luv, NULL);
                        cvMerge(Mll, Ma, Mbb, NULL, Mlab);
                        cvAvgSdv(Mlab, &mean_lab, &std_lab, NULL);
                        cvMerge(Mx, Myy, Mz, NULL, Mxyz);
                        cvAvgSdv(Mxyz, &mean_xyz, &std_xyz, NULL);
                        cvMerge(Muu, Mxx, Mvvv, NULL, Mluxv);
                        cvAvgSdv(Mluxv, &mean_luxv, &std_luxv, NULL);

                        printf("La zona nÃºmero %d:\t\nCorresponde a la clase nÃºmero:", zona);
                        scanf("%d", &b);
                        CvScalar valor_zona;
                        CvScalar valor_zona_gral;

                        // Clase 1: Verde Oscuro (Clase 1)
                        if (b == 1) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 128;
                            valor_zona.val[2] = 0;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        // Clase 2: Verde Medio (Clase 1)
                        else if (b == 2) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 255;
                            valor_zona.val[2] = 0;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        // Clase 3: Verde Amarillo (Clase 1)
                        else if (b == 3) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 217;
                            valor_zona.val[2] = 202;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        // Clase 4: Amarillo PintÃ³n (Clase 2)
                        else if (b == 4) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 255;
                            valor_zona.val[2] = 255;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        // Clase 5: PintÃ³n (Clase 2)
                        else if (b == 5) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 160;
                            valor_zona.val[2] = 218;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        // Clase 6: Maduro PintÃ³n (Clase 3)
                        else if (b == 6) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 100;
                            valor_zona.val[2] = 218;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        //                else if (b==7){
                        //                    valor_zona.val[0] = 70;
                        //                    valor_zona.val[1] = 100;
                        //                    valor_zona.val[2] = 218;
                        //                    valor_zona_gral.val[0] = 0;
                        //                    valor_zona_gral.val[1] = 255;
                        //                    valor_zona_gral.val[2] = 0;

                        //                }
                                        // Clase 7: Maduro Sobremaduro (Clase 4)
                        else if (b == 7) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 0;
                            valor_zona.val[2] = 255;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        //                else if (b==9){
                        //                    valor_zona.val[0] = 255;
                        //                    valor_zona.val[1] = 0;
                        //                    valor_zona.val[2] = 255;
                        //                    valor_zona_gral.val[0] = 0;
                        //                    valor_zona_gral.val[1] = 255;
                        //                    valor_zona_gral.val[2] = 0;
                        //                }
                                        // Clase 8: Secos (Clase 5)
                        else if (b == 8) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 0;
                            valor_zona.val[2] = 128;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 0;
                        }
                        //                else if (b==11){
                        //                    valor_zona.val[0] = 94;
                        //                    valor_zona.val[1] = 94;
                        //                    valor_zona.val[2] = 94;
                        //                    valor_zona_gral.val[0] = 0;
                        //                    valor_zona_gral.val[1] = 255;
                        //                    valor_zona_gral.val[2] = 0;
                        //                }
                                        // Clase 9: Tallo Rama (Clase 6)
                        else if (b == 9) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 50;
                            valor_zona.val[2] = 100;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 255;
                        }
                        //                else if (b==13){
                        //                    valor_zona.val[0] = 0;
                        //                    valor_zona.val[1] = 100;
                        //                    valor_zona.val[2] = 200;
                        //                    valor_zona_gral.val[0] = 0;
                        //                    valor_zona_gral.val[1] = 255;
                        //                    valor_zona_gral.val[2] = 255;
                        //                }
                                        // Clase 10: Pedunculo (Clase 7)
                        else if (b == 10) {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 128;
                            valor_zona.val[2] = 128;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 255;
                            valor_zona_gral.val[2] = 255;
                        }
                        // Clase 11: Hojas (Clase 8)
                        else if (b == 11) {
                            valor_zona.val[0] = 128;
                            valor_zona.val[1] = 128;
                            valor_zona.val[2] = 0;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 128;
                            valor_zona_gral.val[2] = 0;
                        }
                        //                else if (b==16){
                        //                    valor_zona.val[0] = 255;
                        //                    valor_zona.val[1] = 255;
                        //                    valor_zona.val[2] = 0;
                        //                    valor_zona_gral.val[0] = 0;
                        //                    valor_zona_gral.val[1] = 128;
                        //                    valor_zona_gral.val[2] = 0;
                        //                }
                                        // Clase 12: Fondo (Clase 9)
                        else if (b == 12) {
                            valor_zona.val[0] = 208;
                            valor_zona.val[1] = 208;
                            valor_zona.val[2] = 208;
                            valor_zona_gral.val[0] = 255;
                            valor_zona_gral.val[1] = 0;
                            valor_zona_gral.val[2] = 0;
                        }
                        //                else if (b==18){
                        //                    valor_zona.val[0] = 255;
                        //                    valor_zona.val[1] = 0;
                        //                    valor_zona.val[2] = 0;
                        //                    valor_zona_gral.val[0] = 255;
                        //                    valor_zona_gral.val[1] = 0;
                        //                    valor_zona_gral.val[2] = 0;
                        //                }
                        //                else if (b==19){
                        //                    valor_zona.val[0] = 255;
                        //                    valor_zona.val[1] = 255;
                        //                    valor_zona.val[2] = 255;
                        //                    valor_zona_gral.val[0] = 255;
                        //                    valor_zona_gral.val[1] = 255;
                        //                    valor_zona_gral.val[2] = 255;
                        //                }
                        else {
                            valor_zona.val[0] = 0;
                            valor_zona.val[1] = 0;
                            valor_zona.val[2] = 0;
                            valor_zona_gral.val[0] = 0;
                            valor_zona_gral.val[1] = 0;
                            valor_zona_gral.val[2] = 0;
                        }


                        //cvAndS(imgmks1, valor_zona, imgmks1_temp, NULL);
                        cvAndS(imgseg_3canales, valor_zona, imgmks1_temp, NULL);
                        cvAdd(zonas, imgmks1_temp, zonas, NULL);

                        cvNamedWindow("Zonas Etiquetadas1", 0);
                        cvShowImage("Zonas Etiquetadas1", zonas);

                        cvAndS(imgseg_3canales, valor_zona_gral, imgmks1_temp, NULL);
                        cvAdd(zonas_gral, imgmks1_temp, zonas_gral, NULL);

                        cvNamedWindow("Zonas Etiquetadas2", 0);
                        cvShowImage("Zonas Etiquetadas2", zonas_gral);

                        cvAndS(imgmks1, valor_zona, imgmks1_temp, NULL);
                        cvAdd(zonas_ori, imgmks1_temp, zonas_ori, NULL);

                        cvNamedWindow("Zonas Etiquetadas3", 0);
                        cvShowImage("Zonas Etiquetadas3", zonas_ori);

                        //imgmks1
                        cvWaitKey(0);

                        //contador1 = contador1 + 1;

                        Process = fopen("vectores.txt", "a+t");
                        fprintf(Process, "%d\t %s\t %d\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %d\t %d\n", rama,
                            Buffer, b, mean_rgb.val[0], mean_rgb.val[1], mean_rgb.val[2], mean_hsv.val[0], mean_hsv.val[1], mean_hsv.val[2],
                            mean_ycrcb.val[0], mean_ycrcb.val[1], mean_ycrcb.val[2], mean_luv.val[0], mean_luv.val[1], mean_luv.val[2],
                            mean_xyz.val[0], mean_xyz.val[1], mean_xyz.val[2], mean_lab.val[0], mean_lab.val[1], mean_lab.val[2],
                            mean_luxv.val[0], mean_luxv.val[1], mean_luxv.val[2],
                            std_rgb.val[0], std_rgb.val[1], std_rgb.val[2], std_hsv.val[0], std_hsv.val[1], std_hsv.val[2],
                            std_ycrcb.val[0], std_ycrcb.val[1], std_ycrcb.val[2], std_luv.val[0], std_luv.val[1], std_luv.val[2],
                            std_xyz.val[0], std_xyz.val[1], std_xyz.val[2], std_lab.val[0], std_lab.val[1], std_lab.val[2],
                            std_luxv.val[0], std_luxv.val[1], std_luxv.val[2], area_tod, perimeter_tod, centroX, centroY);
                        fclose(Process);


                        cvClearSeq(rgb);
                        cvClearSeq(hsv);
                        cvClearSeq(ycrcb);
                        cvClearSeq(luv);
                        cvClearSeq(lab);
                        cvClearSeq(xyz);
                        cvClearSeq(luxv);

                        cvReleaseMemStorage(&almacenamiento_de_frutos_RGB);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_HSV);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_LUXV);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_XYZ);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_LAB);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_LUV);
                        cvReleaseMemStorage(&almacenamiento_de_frutos_YCrCb);


                        cvReleaseMat(&Mrgb);
                        cvReleaseMat(&Mr);
                        cvReleaseMat(&Mg);
                        cvReleaseMat(&Mb);
                        cvReleaseMat(&Mhsv);
                        cvReleaseMat(&Mh);
                        cvReleaseMat(&Ms);
                        cvReleaseMat(&Mv);
                        cvReleaseMat(&Mycrcb);
                        cvReleaseMat(&My);
                        cvReleaseMat(&Mcr);
                        cvReleaseMat(&Mcb);
                        cvReleaseMat(&Mluv);
                        cvReleaseMat(&Ml);
                        cvReleaseMat(&Mu);
                        cvReleaseMat(&Mvv);
                        cvReleaseMat(&Mlab);
                        cvReleaseMat(&Mll);
                        cvReleaseMat(&Ma);
                        cvReleaseMat(&Mbb);
                        cvReleaseMat(&Mxyz);
                        cvReleaseMat(&Mx);
                        cvReleaseMat(&My);
                        cvReleaseMat(&Mz);
                        cvReleaseMat(&Mluxv);
                        cvReleaseMat(&Muu);
                        cvReleaseMat(&Mxx);
                        cvReleaseMat(&Mvvv);

                        cvZero(imgmks);
                        cvZero(imgseg);
                        cvZero(imgseg_3canales);

                        valor_zona.val[0] = 0;
                        valor_zona.val[1] = 0;
                        valor_zona.val[2] = 0;
                        valor_zona_gral.val[0] = 0;
                        valor_zona_gral.val[1] = 0;
                        valor_zona_gral.val[2] = 0;

                        flag_carga = 0;

                        //cvWaitKey(0);

                    }
                    sprintf(str1, "label_gral_%s", Buffer);
                    sprintf(str2, "label_stru_%s", Buffer);
                    sprintf(str3, "label_orig_%s", Buffer);
                    //sprintf(str4, "marcas_%s", Buffer);
                    sprintf(str5, "wshed_%s", Buffer);
                    cvSaveImage(str1, zonas_gral);
                    cvSaveImage(str2, zonas);
                    cvSaveImage(str3, zonas_ori);
                    //cvSaveImage(str4, marker_mask);
                    cvSaveImage(str5, wshed);

                    strcpy(Buffer2, Buffer);
                    printf("archivo anterior %s\n", Buffer2);

                    cvReleaseMemStorage(&storage2);
                    cvReleaseMemStorage(&storage_poly2);

                    printf("\n\nHa terminado de procesar esta imagen. \n"
                        "Presione 'f' para continuar\n\n");

                }
                else
                    printf("else PUNTO PARA PROCESO DE SECUENCIAS POR SEGMENTO\n");




            }

        }


    loop:

        cvZero(marker_mask);
        cvZero(marker_mask2);
        cvZero(mascara);
        cvZero(markers);
        cvZero(markers_8bits);
        cvZero(img0);
        cvZero(img_inicial);
        cvZero(img);
        cvZero(img_gray);
        cvZero(wshed);
        cvZero(L);
        cvZero(U);
        cvZero(X);
        cvZero(V);
        cvZero(imgluxv);
        cvZero(r_plane);
        cvZero(g_plane);
        cvZero(b_plane);
        cvZero(r_plane_marca);
        cvZero(g_plane_marca);
        cvZero(b_plane_marca);
        cvZero(imghsv);
        cvZero(imgray);
        cvZero(imgxyz);
        cvZero(imglab);
        cvZero(imgluv);
        cvZero(imgycc);
        cvZero(imgseg);
        cvZero(imgmks);
        cvZero(imgmks_copia);
        cvZero(imgmks1);
        cvZero(imgmks2);
        cvZero(r_plane_seg);
        cvZero(g_plane_seg);
        cvZero(b_plane_seg);
        cvZero(zonas);
        cvZero(zonas_gral);
        cvZero(zonas_ori);
        cvZero(imgmks1_temp);
        cvZero(imgseg_3canales);

        cvReleaseImage(&marker_mask);
        cvReleaseImage(&marker_mask2);
        cvReleaseImage(&mascara);
        cvReleaseImage(&markers);
        cvReleaseImage(&markers_8bits);
        cvReleaseImage(&img0);
        cvReleaseImage(&img_inicial);
        cvReleaseImage(&img);
        cvReleaseImage(&img_gray);
        cvReleaseImage(&wshed);
        cvReleaseImage(&L);
        cvReleaseImage(&U);
        cvReleaseImage(&X);
        cvReleaseImage(&V);
        cvReleaseImage(&imgluxv);
        cvReleaseImage(&r_plane);
        cvReleaseImage(&g_plane);
        cvReleaseImage(&b_plane);
        cvReleaseImage(&r_plane_marca);
        cvReleaseImage(&g_plane_marca);
        cvReleaseImage(&b_plane_marca);
        cvReleaseImage(&imghsv);
        cvReleaseImage(&imgray);
        cvReleaseImage(&imgxyz);
        cvReleaseImage(&imglab);
        cvReleaseImage(&imgluv);
        cvReleaseImage(&imgycc);
        cvReleaseImage(&imgseg);
        cvReleaseImage(&imgmks);
        cvReleaseImage(&imgmks_copia);
        cvReleaseImage(&imgmks1);
        cvReleaseImage(&imgmks1_temp);
        cvReleaseImage(&imgmks2);
        cvReleaseImage(&r_plane_seg);
        cvReleaseImage(&g_plane_seg);
        cvReleaseImage(&b_plane_seg);
        cvReleaseImage(&zonas);
        cvReleaseImage(&zonas_gral);
        cvReleaseImage(&zonas_ori);
        cvReleaseImage(&imgseg_3canales);

        cvReleaseMemStorage(&storage);

        cvDestroyAllWindows();


    }
    return 1;
}

#ifdef _EiC
main(1, "watershed_caracteristics.c");
#endif

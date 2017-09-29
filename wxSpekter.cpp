/////////////////////////////////////////////////////////////////////////////
// Ime:			vs/wxSpekter/wxSpekter.cpp
// Namen:		Izraèun parametrov moènostnega spektra BPSK in 
//				dBi/dBd, eff./peak/mean šuma.
// Autor:		B.Nagliè
// Osnova:		/sample/drawing.cpp
// Datum:		2014.08.22b
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
//#include "time.h"
//#include "math.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//
//#ifndef WX_PRECOMP
//    #include "wx/app.h"
//    #include "wx/frame.h"
//    #include "wx/menu.h"
//    #include "wx/msgdlg.h"
//    #include "wx/log.h"
//    #include "wx/textctrl.h"
//    #include "wx/textdlg.h"
//#endif
//

#include "wx/colordlg.h"
#include "wx/image.h"
#include "wx/artprov.h"
#include "wx/dcbuffer.h"
#include "wx/dcgraph.h"
#include "wx/overlay.h"
#include "wx/graphics.h"
#include "wx/filename.h"
#include "wx/metafile.h"

#define TEST_CAIRO_EVERYWHERE 0

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon
#ifndef wxHAS_IMAGES_IN_RESOURCES
    //#include "../sample.xpm"
	#include "../spekter.xpm"
#endif


// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

#define C_8 2.99792458 //c*1.0e-8 - mantisa
//#define ENASTRAD (M_PI / 180.0)

BOOL    m_bAutoRef = FALSE; 
BOOL	m_bBreak = FALSE;
int		m_nDinam = 1;
double	m_dAnap  = 0.001;  //amplituda signala v V
double	m_dNo    = 2.0e-19; //gostota moèi šuma No v W
double	m_dPomik = 0;
double	m_nSwing = 1;
double	m_dIFB   = 1.0;      //razmerje RBW/B. B=10MHz

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

    virtual int OnExit() { return 0; }

protected:
};

class MyCanvas;

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnPomoc(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnShow(wxCommandEvent &event);
    void OnAutoRef(wxCommandEvent& event);
    void OnSumPlus(wxCommandEvent& event);
    void OnSumMinus(wxCommandEvent& event);
    //void OnFrek(wxCommandEvent& event);
    void OnNapPlus(wxCommandEvent& event);
    void OnNapMinus(wxCommandEvent& event);
    //void OnCheckDinam(wxCommandEvent& event);
    void OnDinam(wxCommandEvent& event);
    //void OnBreak(wxCommandEvent& event);
    void OnBWPlus(wxCommandEvent& event);
    void OnBWMinus(wxCommandEvent& event);

	//void OnCheckMenuItem(wxCommandEvent& event);
    //void OnUpdateCheckMenuItemUI(wxUpdateUIEvent& event);
    //void OnUpdateCheckDinamUI(wxUpdateUIEvent& event);

    //wxMenuItem *GetLastMenuItem() const;

	//static inline double DegToRad(double deg) { return (deg * M_PI) / 180.0; }

#if wxUSE_COLOURDLG
    wxColour SelectColour();
#endif // wxUSE_COLOURDLG
    void PrepareDC(wxDC& dc);

    int         m_backgroundMode;
    int         m_textureBackground;
    wxMappingMode m_mapMode;
    double      m_xUserScale;
    double      m_yUserScale;
    int         m_xLogicalOrigin;
    int         m_yLogicalOrigin;
    bool        m_xAxisReversed,
                m_yAxisReversed;
    wxColour    m_colourForeground,    // these are _text_ colours
                m_colourBackground;
    wxBrush     m_backgroundBrush;
    MyCanvas   *m_canvas;

private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

// define a scrollable canvas for drawing onto
class MyCanvas: public wxScrolledWindow
{
public:
    MyCanvas( MyFrame *parent );

    void OnPaint(wxPaintEvent &event);
    //void OnMouseMove(wxMouseEvent &event);
    //void OnMouseDown(wxMouseEvent &event);
    //void OnMouseUp(wxMouseEvent &event);

    void ToShow(int show) { m_show = show; Refresh(); }
    void UseBuffer(bool use) { m_useBuffer = use; Refresh(); }
    void Draw(wxDC& dc);

protected:
    enum DrawMode
    {
        Draw_Normal,
        Draw_Stretch
    };

    void DrawSpekter(wxDC& dc);

private:
    MyFrame *m_owner;

    int          m_show;
    wxBitmap     m_smile_bmp;
    wxIcon       m_std_icon;
    bool         m_clip;
    wxOverlay    m_overlay;
    bool         m_rubberBand;
    wxPoint      m_anchorpoint;
    wxPoint      m_currentpoint;
#if wxUSE_GRAPHICS_CONTEXT
    bool         m_useContext ;
#endif
    bool         m_useBuffer;

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    File_Quit = wxID_EXIT,
    File_About = wxID_ABOUT,

    MenuShow_First = wxID_HIGHEST,
    File_ShowDefault = MenuShow_First,
    File_Copy,
    MenuShow_Last = File_Copy,

    MenuOption_First,

    MapMode_AutoRef = MenuOption_First,
    MapMode_NPlus,
    MapMode_NMinus,
    MapMode_UPlus,
    MapMode_UMinus,
    //Menu_Menu_Check,
    MapMode_BWPlus,
    MapMode_BWMinus,
    //MapMode_Prekini,
    //MapMode_Pomik,
    MapMode_Dinam,

    MenuPomoc_First,

    Pomoc_Pomoc = MenuPomoc_First,
    Pomoc_O,

    MenuPomoc_Last = Pomoc_O,

    MenuOption_Last = Pomoc_O
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    // Create the main application window
    MyFrame *frame = new MyFrame(wxT("Spekter"), wxDefaultPosition, wxSize(960, 765));

    // Show it
    frame->Show(true);
#if wxUSE_LIBPNG
      wxImage::AddHandler( new wxPNGHandler );
#endif

    return true;
}

// ----------------------------------------------------------------------------
// MyCanvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them.
wxBEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
    EVT_PAINT  (MyCanvas::OnPaint)
    //EVT_MOTION (MyCanvas::OnMouseMove)
    //EVT_LEFT_DOWN (MyCanvas::OnMouseDown)
    //EVT_LEFT_UP (MyCanvas::OnMouseUp)
wxEND_EVENT_TABLE()

//#include "../smile.xpm"

MyCanvas::MyCanvas(MyFrame *parent)
        : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    m_owner = parent;
    m_show = File_ShowDefault;
    
	//m_smile_bmp = wxBitmap(smile_xpm);
   
	m_std_icon = wxArtProvider::GetIcon(wxART_INFORMATION);

    m_clip = false;
    m_rubberBand = false;
#if wxUSE_GRAPHICS_CONTEXT
    m_useContext = false;
#endif
    m_useBuffer = false;
}

void MyCanvas::DrawSpekter(wxDC& dc)
{
	int		xi0=50, yi0=140, xi1=800, yi1=500, xia=0, xib=0, yia=0, yib=0, width=1;
	int		i=0, j=0, n=0, korak=1, nXm=180, nB2st=0;
	int		nXPmax=0, nYPmax=0, nXPmax2=0, nYPmax2=0, nXPnyq1=0, nYPnyq1=0, nXPnyq2=0, nYPnyq2=0;
	double	x=0, u=0, sxzx=0, p=0, p1=0, dDx=0, dYsc=8.0, dRef=0, dDin=0, dEpsilon=0;
	double	dA=0.0, dPn=0, dP=0, dPmax1=0, dPmax2=0, dPnyq1=0, dPnyq2=0, dN=0, dNpov=0, dR=50;
	double	dSumP1=0, dSumP2=0, dSum=0, dPNny=0, d1stRad=0;
	double	dPY=0, dPtot=0, dPny=0, dPost=0, dB=0, dB2st=0, dB2rad=0;
	double	dAny=0, dAost=0; //razmerja Pny/Ptot, Pny/Ptot v dB
	double	dNlinB=0, dNlin=0, dNlog=0, dNlinpov=0, dNlineff=0, dNlogpov=0, dNlogeff=0, dNmax=0;
	wxString text;

    //dc.SetFont( *wxSWISS_FONT );
    dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetPen( wxPen( *wxBLACK, width ) );
    dc.SetFont( wxFont(12, wxMODERN, wxNORMAL, wxBOLD, false) );
	dc.DrawText(wxT("BPSK signal 10 Mb/s z dodanim šumom na vhodu sprejemnika "), xi0 - 22, 15);
    dc.SetFont( wxFont(9, wxMODERN, wxNORMAL, wxNORMAL, false) );
	dc.SetTextForeground( *wxBLACK );
	dc.SetBrush( *wxTRANSPARENT_BRUSH );

	nXm = 1100; yi1 = (int)(60 * dYsc); korak = 1; dDx = +810.; dDin = 60.0; //korak x je 3st., zamik x je +200st., 
	dA = m_dAnap; dR = 50.0; dB = 10.0e6; // B=10MHz
	d1stRad = M_PI / 180.0; dEpsilon = d1stRad; //1 stopinja v rad
	p = (dA*dA)/(2*dR); //Skupna efektivna vrednost moèi P = Ptot
	if(p > 1.e-15) dPtot = 10. * log10(p) + 30.0; //Ptot v dBm, lim -120dBm
	else           dPtot = -120.;
	if(m_bAutoRef == TRUE) dRef = floor((dPtot - 1) / 10.0) * 10.0 + 20.0; //v dBm
	else                   dRef = -40;
	dPmax1 = dRef - dDin; dPmax2 = dRef - dDin; dPnyq1 = dRef - dDin; dPnyq2 = dRef - dDin; //reset spomina maks. vrednosti
	dB2st = 90.0 * m_dIFB; nB2st = (int)dB2st; //polovièna B v stopinjah: 90 stopinj = 10MHz/2
	dB2rad = d1stRad * dB2st; //B/2 v rad
	dPn = m_dNo * dB; //vršna vrednost dodanega šuma v B=10MHz: Pn = No * B
	dSum = 0; dSumP1 = 0; dSumP2 = 0; dNmax = 0;
	n = 1000000; //izraèun vrednosti šuma
	for (i=0; i<n; i++) {
		dNlin = dPn*((double)((rand() % 1000) - 1) / 1000.0); //linearna vrednost trenutnega šuma
		if(dNlin > 1.e-15) dNlog = 10*log10(dNlin)+30.0; //log. vrednost trenutnega suma v dBm
		else               dNlog = -120.0;
		dSum   += pow(dNlin, 2); //kvadrat Gauss-ovega šuma
		dSumP1 += dNlin; //lin. sum
		dSumP2 += dNlog; //log. sum
		if(dNlin > dNmax) dNmax = dNlin;
	}
	dNlinpov = dSumP1 / ((double)n); //povpreèna vrednost lin. moèi
	dNlogpov = dSumP2 / ((double)n); //povpreèna vrednost log. moci
	dNlineff = sqrt(dSum / ((double)n)); //efektivna vrednost šuma v B=10MHz za n=5000
	dNlogeff = 10.0*log10(dNlineff)+30.0; //log. vrednost lin eff. moci v dBm
	dNlinB   = dNlineff * m_dIFB; //efektivna vrednost lin. moèi šuma v B

	//Izracun razmerja osnovnega proti ostalim listom - samo desna stran
	dSumP1 = 0.0; //Samo osnovni - integral od 0 do B/2 po dx=1 stopinja
	for(i=0; i<nB2st; i++) {
		x = d1stRad*((double)(i)+0.5); //+0.5 za sredino stopnice
		if(x==0) u = 1.0;
		else     u = sin(x)/x;
		dSumP1 += (u*u);
	}
	dSumP1 *= (d1stRad); //krat dx v rad
	dSumP2  = 0.0; //Ostali - integral od B/2 do 10000*pi po dx=1 stopinja
	for(j=0; j<10000; j++) {
		dSum = 0;
		for(i=0; i<180.0; i++) {
			x = d1stRad*((double)(nB2st+j*180.0+i)+0.5); //+0.5 za sredino stopnice
			if(x==0) u = 1.0;
			else     u = sin(x)/x;
			dSum += (u*u);
		}
		dSum *= (d1stRad); //krat dx v rad
		dSumP2 += dSum;
	}

	//Izraèun dBi/dBd po stopnicah: dx * sum(f((x1+x2)/2))
	dSum = 0.0;
	for(i=0; i<360; i++) { //integral od 0 do 2pi po 1 stopinjo
		x = d1stRad*((double)(i)+0.5); //+0.5 za sredino stopnice
		if(x > 0) u = (1 - cos(x)) / x;
		else      u = 0.0;
		dSum += u;
	}
	dSum *= (d1stRad); //integral((1-cos(x))/x, 0, 2pi) krat dx = 1 stopinja v rad
	dSum = 12.0 / (C_8 * dSum); //Gdipol = (4) / integral((1-cos(x))/x, 0, 2pi)

	dAny  = dSumP1 / (dSumP1 + dSumP2); // P(B)/Ptot
	dAost = dSumP2 / (dSumP1 + dSumP2); // Post/Ptot
	dPny  = dPtot + 10.0 * log10(dAny);  //ef. vrednost P(B) moèi v dBm
	dPost = dPtot + 10.0 * log10(dAost); //ef. vrednost ostale moèi v dBm
	dPNny = 10.0 * log10(dNlinB) + 30.0; //ef. vrednost moèi šuma v dBm v B

	dc.SetPen(*wxBLACK_PEN); //Izpis vhodnih parametrov
    dc.DrawText( "signal:", xi0 - 22, 40 );
    text.Printf( wxT("A = %5.2f mV (%4.1f ohm), No = %6.1f dBm/Hz"), dA*1.e3, dR, 10*log10(m_dNo*1.0e3) );
	dc.DrawText( text, xi0 + 50, 40 );
    text.Printf( wxT("Ref = %5.1f dBm, B = %5.1f MHz"), dRef, 10.0*m_dIFB );
	dc.DrawText( text, xi0 + 50, 55 ); 
	//Izpis razmerij moèi za +-B/2 proti ostalim
	dc.DrawText( "razmerje:", xi0 - 22, 70 );
    text.Printf( wxT("P(+-B/2)/Ptot = %5.2f dB (%4.2f %%)"), 10.*log10(dAny),  dAny*100. );
	dc.DrawText( text, xi0 + 50, 70 );
	text.Printf( wxT("Post/Ptot     = %5.2f dB (%4.2f %%)"), 10.*log10(dAost), dAost*100. );
	dc.DrawText( text, xi0 + 50, 85 );
	//Izpis razliènih moèi spektra in S/N
	text.Printf( wxT("Ne(B) = %6.2f dBm"), dPNny);
	dc.DrawText( text, xi0 + 390, 40 );
	text.Printf( wxT("Pe    = %6.2f dBm"), dPtot);
	dc.DrawText( text, xi0 + 390, 55 );
	text.Printf( wxT("Pe(B) = %6.2f dBm"), dPny);
	dc.DrawText( text, xi0 + 390, 70 );
	text.Printf( wxT("Post  = %6.2f dBm"), dPost);
	dc.DrawText( text, xi0 + 390, 85 );
	text.Printf( wxT("S/N   = %6.2f dB"),  dPny - dPNny ); 
	dc.DrawText( text, xi0 + 390, 100 );

	//Izpis Giso in eff/maks/povpr moèi
	text.Printf( wxT("(G dipola)    = %5.3f dBi (%8.6f)"), 10.0*log10(dSum), dSum);
	dc.DrawText( text, xi0 + 560, 40 );
	text.Printf( wxT("Neff   = %6.2f dBm"), dNlogeff);
	dc.DrawText( text, xi0 + 560, 55 );
	text.Printf( wxT("Nmax   = %6.2f dBm (eff/max%6.2f dB)"), 10.0*log10(dNmax)+30.0, dNlogeff-(10.0*log10(dNmax)+30.0));
	dc.DrawText( text, xi0 + 560, 70 );
	text.Printf( wxT("<Nlin> = %6.2f dBm (lin/eff%6.2f dB)"), 10.0*log10(dNlinpov)+30.0, 10.0*log10(dNlinpov/dNlineff));
	dc.DrawText( text, xi0 + 560, 85 );
	text.Printf( wxT("<Nlog> = %6.2f dBm (log/eff%6.2f dB)"), dNlogpov, dNlogpov-dNlogeff);
	dc.DrawText( text, xi0 + 560, 100 );
 
	//Izris frekvenènega spektra moèi v dBm
	for(n=0; n < m_nDinam; n++) { //Število opletov
		dc.SetPen(*wxBLACK_PEN); //èrno pero
		dc.DrawText( wxT("P[dBm]"), xi0 - 22, yi0 - 22 );
		dc.SetBrush( *wxWHITE_BRUSH );
		dc.DrawRectangle(xi0, yi0, xi1 + 1, yi1 + 1); //brisi diagram
		dc.SetBrush( *wxTRANSPARENT_BRUSH );
		dc.SetPen( wxPen( *wxLIGHT_GREY, 1, wxPENSTYLE_DOT));
		for(i=((int)(dRef - 60)); i <= ((int)dRef); i+=5) { //y-mreža po 5dB
			//yia = (int)((i - dRef) * dYsc + 0.5);
			yia = (int)((i - dRef + dDin) * dYsc + 0.5);
			if((i % 5) == 0) dc.DrawLine(xi0 - 5, yi0 + yi1 - yia, xi0 + xi1 + 6, yi0 + yi1 - yia);
			dc.DrawLine(xi0 - 3, yi0 + yi1 - yia, xi0, yi0 + yi1 - yia);
			dc.DrawLine(xi0 + xi1, yi0 + yi1 - yia, xi0 + xi1 + 4, yi0 + yi1 - yia);
			if((i % 10) == 0) {
				text.Printf( wxT("%3d"), i);
				dc.DrawText( text, xi0 + xi1 + 10, yi0 + yi1 - yia - 7 );
				dc.DrawText( text, xi0 - 30, yi0 + yi1 - yia - 7 );
			}
		}
		dc.DrawText( wxT("frekvenca[MHz]"), xi0 + xi1/2 - 50, yi0 + yi1 + 25 );
		for(i=0; i <= nXm/90; i++) { //x-mreža po pi/2 (5MHz)
			xia = (int)floor(((double)i * 90. * ((double)xi1 / nXm)) + 0.5);
			dc.DrawLine(xi0 + xia, yi0 + yi1 + 6, xi0 + xia, yi0);
			if(((i+1) % 2) == 0) {
				text.Printf( wxT("%2d"), (-45 + i*5) );
				dc.DrawText( text, xi0 + xia - 10, yi0 + yi1 + 8 );
			}
		}
		dc.SetPen( wxPen( *wxBLACK, 1, wxPENSTYLE_SOLID));
		xia = 0; yia = 0;
		nXPnyq1=0; nXPnyq2=0;
		for(i=0; i<=nXm; i++) { //Izris spektra po koraku
			x = d1stRad * ((double)i * korak - dDx);
			if(x == 0) sxzx = 1.0;
			else     sxzx = sin(x) / x;
			u = dA * sxzx; //Signal z ampl. A
			p1 = (u*u)/(2.*dR) * (1.0); // ef. vrednost moèi, faktor max/tot=1
			for(j=0; j<15; j++) { //Šum - latenca za dinamièni prikaz
				dN = dPn*((double)((rand() % 50) - 1) / 50.); //Gauss-ov šum v B=10MHz 
				//p = ((u*u)/(2.*dR) * (dAny / dAmax) + dN) * m_dIFB; // faktor max/tot, dodan sum z ampl Pn
				//p = ((u*u)/(2.*dR) * (1.0) + dN); // faktor max/tot=1, dodan šum z ampl Pn
				p = p1 + dN; // dodan šum z ampl Pn
				if(p > 1.e-15) dP = 10  * log10(p) + 30.0; //P v dBm, lim -120dBm
				else           dP = -120 ;
				if(dP > (dRef - 60.0)) dPY = (dP - dRef + dDin) * dYsc; //lim. spodnje meje grafa
				else                   dPY = 0.0;
				xib = (int)floor((double)i * xi1 / nXm + 0.5); 
				yib = (int)floor(dPY + 0.5); //nova toèka
				if(yib > yi1) yib = yi1; //limita max
				if(yib < 0)   yib =   0; //limita min
				if(i == 0) dc.DrawPoint(xi0+xib, yi0+yi1-yib);
				else       dc.DrawLine(xi0+xia, yi0+yi1-yia, xi0+xib, yi0+yi1-yib);
				xia = xib; yia = yib; //shrani staro toèko
			}
			if((abs(x + dB2rad) < dEpsilon) && (dP > dPnyq1)) { //1. marker +B/2
				dPnyq1 = dP; nXPnyq1 = xib; nYPnyq1 = yib; 
			}
			if((abs(x - dB2rad) < dEpsilon) && (dP > dPnyq2)) { //2. marker -B/2
				dPnyq2 = dP; nXPnyq2 = xib; nYPnyq2 = yib; 
			}
			if(nXPnyq1 == 0) nXPnyq1 = 0; //lim., ce je marker zunaj podroèja
			if(nXPnyq2 == 0) nXPnyq2 = xi1;
			if((x < -M_PI) && (dP > dPmax2)) { 
				dPmax2 = dP; nXPmax2 = xib; nYPmax2 = yib; //išèi 2. max
			}
			if(dP > dPmax1) { dPmax1 = dP; nXPmax = xib; nYPmax = yib; } //išèi 1. maks
		}
		//Izris markerjev
		dc.SetPen( wxPen( *wxRED, 1, wxPENSTYLE_SOLID));
		dc.SetTextForeground( *wxRED );
		dc.DrawLine(xi0+nXPmax,  yi0+yi1-nYPmax-6, xi0+nXPmax, yi0+yi1-nYPmax+6); //marker za 1. max
		dc.DrawText( "1", xi0 + nXPmax  + 2, yi0 + yi1 - nYPmax  - 20 );
		dc.DrawLine(xi0+nXPmax2, yi0+yi1-nYPmax2-6, xi0+nXPmax2, yi0+yi1-nYPmax2+6); //marker za 2. max
		dc.DrawText( "4", xi0 + nXPmax2 + 2, yi0 + yi1 - nYPmax2 - 20 );
		dc.SetPen( wxPen( *wxRED, 1, wxPENSTYLE_SHORT_DASH));
		dc.DrawLine(xi0+nXPnyq1, yi0+yi1, xi0+nXPnyq1, yi0); //1. marker za B/2
		dc.DrawText( "2", xi0 + nXPnyq1 + 2, yi0 + yi1 - nYPnyq1 - 20 );
		dc.DrawLine(xi0+nXPnyq2, yi0+yi1, xi0+nXPnyq2, yi0); //2. marker za B/2
		dc.DrawText( "3", xi0 + nXPnyq2 + 2, yi0 + yi1 - nYPnyq2 - 20 );
		dc.SetPen( wxPen( *wxBLACK, 1, wxPENSTYLE_SOLID));
		dc.SetTextForeground( *wxBLACK );

		//if(m_bBreak == TRUE) break;
		for(i=0; i<10000000; i++) for(j=0; j<100000; j++) ; //wait
	}
	//if(m_bBreak == TRUE) break;

	//Izris nivoja celotne moèi P
	dc.SetPen( wxPen( *wxBLUE, 1, wxPENSTYLE_SHORT_DASH));
	dc.SetTextForeground( *wxBLUE );
	yia = (int)floor((dPtot - dRef + dDin) * dYsc + 0.5);
	dc.DrawLine(xi0 - 0, yi0 + yi1 - yia, xi0 + xi1 + 0, yi0 + yi1 - yia);
	text.Printf( wxT("Pe"));
	dc.DrawText( text, xi0 + xi1 + 37, yi0 + yi1 - yia - 9 );
	//Izris nivoja moèi v B
	dc.SetPen( wxPen( *wxBLUE, 1, wxPENSTYLE_DOT_DASH));
	yia = (int)floor((dPny - dRef + dDin) * dYsc + 0.5);
	dc.DrawLine(xi0+nXPnyq1, yi0 + yi1 - yia, xi0+nXPnyq2, yi0 + yi1 - yia);
	text.Printf( wxT("Pe(B)"));
	dc.DrawText( text, xi0 + xi1 + 37, yi0 + yi1 - yia - 6 );
	//Izris nivoja ostale moèi izven B
	yia = (int)floor((dPost - dRef + dDin) * dYsc + 0.5);
	dc.DrawLine(xi0 - 0, yi0 + yi1 - yia, xi0+nXPnyq1, yi0 + yi1 - yia);
	dc.DrawLine(xi0+nXPnyq2 - 0, yi0 + yi1 - yia, xi0 + xi1 + 0, yi0 + yi1 - yia);
	text.Printf( wxT("Post"));
	dc.DrawText( text, xi0 + xi1 + 37, yi0 + yi1 - yia - 6 );
	//Izris nivoja moèi šuma v B
	dc.SetPen( wxPen( *wxBLUE, 1, wxPENSTYLE_DOT));
	yia = (int)floor((dPNny - dRef + dDin) * dYsc + 0.5);
	dc.DrawLine(xi0+nXPnyq1, yi0 + yi1 - yia, xi0+nXPnyq2, yi0 + yi1 - yia);
	text.Printf( wxT("Ne(B)"));
	dc.DrawText( text, xi0 + xi1 + 37, yi0 + yi1 - yia - 6 );
	dc.SetPen(*wxBLACK_PEN);
	//Izpis podatkov markerjev
	dc.SetTextForeground( *wxRED );
	text.Printf( wxT("1 (pk): %5.2f dBm"), dPmax1);
	dc.DrawText( text, xi0 + 10, yi0 +  10 );
	text.Printf( wxT("2 (pk): %5.2f dBm (%6.2f dBc)"), dPnyq2, dPnyq2-dPmax1);
	dc.DrawText( text, xi0 + 10, yi0 + 25 );
	text.Printf( wxT("3 (pk): %5.2f dBm (%6.2f dBc)"), dPnyq1, dPnyq1-dPmax1);
	dc.DrawText( text, xi0 + 10, yi0 + 40 );
	text.Printf( wxT("4 (pk): %5.2f dBm (%6.2f dBc)"), dPmax2, dPmax2-dPmax1);
	dc.DrawText( text, xi0 + 10, yi0 + 55 );
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
    if ( m_useBuffer )
    {
        wxBufferedPaintDC bpdc(this);
        Draw(bpdc);
    }
    else
    {
        wxPaintDC pdc(this);
        Draw(pdc);
    }
}

void MyCanvas::Draw(wxDC& pdc)
{
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC gdc;
    wxGraphicsRenderer* const renderer = wxGraphicsRenderer::
#if TEST_CAIRO_EVERYWHERE
        GetCairoRenderer()
#else
        GetDefaultRenderer()
#endif
        ;

    wxGraphicsContext* context;
    if ( wxPaintDC *paintdc = wxDynamicCast(&pdc, wxPaintDC) )
    {
        context = renderer->CreateContext(*paintdc);
    }
    else if ( wxMemoryDC *memdc = wxDynamicCast(&pdc, wxMemoryDC) )
    {
        context = renderer->CreateContext(*memdc);
    }
#if wxUSE_METAFILE && defined(wxMETAFILE_IS_ENH)
    else if ( wxMetafileDC *metadc = wxDynamicCast(&pdc, wxMetafileDC) )
    {
        context = renderer->CreateContext(*metadc);
    }
#endif
    else
    {
        wxFAIL_MSG( "Unknown wxDC kind" );
        return;
    }

    gdc.SetGraphicsContext(context);

    wxDC &dc = m_useContext ? (wxDC&) gdc : (wxDC&) pdc ;
#else
    wxDC &dc = pdc ;
#endif

    PrepareDC(dc);

    m_owner->PrepareDC(dc);

    dc.SetBackgroundMode( m_owner->m_backgroundMode );
    if ( m_owner->m_backgroundBrush.IsOk() ) dc.SetBackground( m_owner->m_backgroundBrush );
    if ( m_owner->m_colourForeground.IsOk() ) dc.SetTextForeground( m_owner->m_colourForeground );
    if ( m_owner->m_colourBackground.IsOk() ) dc.SetTextBackground( m_owner->m_colourBackground );

    if ( m_owner->m_textureBackground) {
        if ( ! m_owner->m_backgroundBrush.IsOk() ) {
            dc.SetBackground(wxBrush(wxColour(0, 128, 0)));
        }
    }

	dc.Clear();

    if ( m_owner->m_textureBackground )
    {
        dc.SetPen(*wxMEDIUM_GREY_PEN);
        for ( int i = 0; i < 200; i++ )
            dc.DrawLine(0, i*10, i*10, 0);
    }

    switch ( m_show )
    {
        case File_ShowDefault:
            DrawSpekter(dc);
            break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU      (File_Quit,		MyFrame::OnQuit)
    EVT_MENU      (File_Copy,		MyFrame::OnCopy)

    EVT_MENU	(MapMode_AutoRef,	MyFrame::OnAutoRef)
    EVT_MENU	(MapMode_NPlus,		MyFrame::OnSumPlus)
    EVT_MENU	(MapMode_NMinus,	MyFrame::OnSumMinus)
    EVT_MENU	(MapMode_UPlus,		MyFrame::OnNapPlus)
    EVT_MENU	(MapMode_UMinus,	MyFrame::OnNapMinus)
    EVT_MENU	(MapMode_BWPlus,	MyFrame::OnBWPlus)
    EVT_MENU	(MapMode_BWMinus,	MyFrame::OnBWMinus)
    //EVT_MENU	(MapMode_Prekini,	MyFrame::OnBreak)
    //EVT_MENU	(MapMode_Pomik,		MyFrame::OnFrek)
    //EVT_MENU	(MapMode_Dinam,		MyFrame::OnCheckDinam)
    EVT_MENU	(MapMode_Dinam,		MyFrame::OnDinam)
    //EVT_MENU(Menu_Menu_Check,     MyFrame::OnCheckMenuItem)

    EVT_MENU	(Pomoc_Pomoc,		MyFrame::OnPomoc)
    EVT_MENU    (Pomoc_O,			MyFrame::OnAbout)

	//EVT_UPDATE_UI(Menu_Menu_Check, MyFrame::OnUpdateCheckMenuItemUI)
	//EVT_UPDATE_UI(MapMode_Dinam, MyFrame::OnUpdateCheckDinamUI)

    //EVT_MENU_RANGE(MenuShow_First,	MenuShow_Last,   MyFrame::OnShow)
    EVT_MENU_RANGE(MenuShow_First,	MenuShow_Last,   MyFrame::OnShow)
wxEND_EVENT_TABLE()

// frame constructor
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size,
                 wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    // set the frame icon
    //SetIcon(wxICON(sample));
    SetIcon(wxICON(spekter));

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(File_ShowDefault, wxT("&Spekter\tF1"), wxT("Prikaz BPSK in šuma"));
#if wxUSE_METAFILE && defined(wxMETAFILE_IS_ENH)
    menuFile->Append(File_Copy, wxT("Kopiraj na <clipboard>\tCtrl-C"));
#endif
    menuFile->AppendSeparator();
    menuFile->Append(File_Quit, wxT("&Izhod\tAlt-X"), wxT("Izhod"));

    wxMenu *menuMapMode = new wxMenu;
    menuMapMode->Append( MapMode_AutoRef, wxT("&Avto Ref +\tF2"),  wxT("Vklop/izklop avtomatske izbire y-osi na -10dB"), true );
    menuMapMode->Append( MapMode_NPlus, wxT("Nivo šuma +\tF4"),  wxT("Poveèanje vrš. vred. gostote moèi šuma za 3dB") );
    menuMapMode->Append( MapMode_NMinus, wxT("Nivo šuma -\tF3"), wxT("Zmanjšanje vrš. vred. gostote moèi šuma za 3dB") );
    menuMapMode->Append( MapMode_UPlus, wxT("Amplituda +\tF6"),  wxT("Poveèanje amplitude signala za 2dB") );
    menuMapMode->Append( MapMode_UMinus, wxT("Amplituda -\tF5"), wxT("Zmanjšanje amplitude signala za 2dB") );
    menuMapMode->Append( MapMode_BWPlus, wxT("IF BW +\tF8"),  wxT("Poveèanje IF pasovne širine s faktorjem 1/2/5") );
    menuMapMode->Append( MapMode_BWMinus, wxT("IF BW -\tF7"), wxT("Zmanjšanje IF pasovne širine z deliteljem 5/2/1") );
    //menuMapMode->Append( MapMode_Prekini, wxT("&Prekini \tF9"), wxT("Prekinitev dinamiènega prikaza") );
    //menuMapMode->Append( MapMode_Pomik, wxT("&Pomik frekvence"), wxT("Vklop/izklop fr. pomika za preizkus hitrosti"), true );
    menuMapMode->Append( MapMode_Dinam, wxT("&Dinamièen prikaz"), wxT("Vklop/izklop zanke za prikaz dinamike šuma"), true);
    wxMenu *menuPomoc = new wxMenu;
    menuPomoc->AppendSeparator();
    menuPomoc->Append( Pomoc_Pomoc, wxT("&Pomoè\tCtrl-H"), wxT("Pomoè za uporabo programa Spekter"));
    menuPomoc->AppendSeparator();
    menuPomoc->Append( Pomoc_O, wxT("&O programu\tCtrl-A"), wxT("O programu Spekter"));
    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, wxT("&Diagram"));
    menuBar->Append(menuMapMode, wxT("&Nastavitve"));
    menuBar->Append(menuPomoc, wxT("&Pomoè"));
    // ... and attach this menu bar to the frame

	//menuBar->Check(Menu_Menu_Check, false);
	//menuBar->Check(MapMode_Pomik, false);
	menuBar->Check(MapMode_Dinam, false);

    SetMenuBar(menuBar);

#if wxUSE_STATUSBAR
    CreateStatusBar(2);
    SetStatusText(wxT("Program Spekter uporablja wxWidgets grafiène knjižnice!"));
#endif // wxUSE_STATUSBAR

    m_mapMode = wxMM_TEXT;
    //m_mapMode = wxMM_METRIC;   //za resolucijo 1mm.
    //m_mapMode = wxMM_LOMETRIC; //za resolucijo 0,1mm.
    m_xUserScale = 1.0;
    m_yUserScale = 1.0;
    m_xLogicalOrigin = 0;
    m_yLogicalOrigin = 0;
    m_xAxisReversed =
    m_yAxisReversed = false;
    m_backgroundMode = wxSOLID;
    m_colourForeground = *wxBLACK;
    m_colourBackground = *wxLIGHT_GREY;
    m_textureBackground = false;

    m_canvas = new MyCanvas( this );
    m_canvas->SetScrollbars( 10, 10, 100, 240 );
}

// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( wxT("Poizkusni program za prikaz BPSK spektra in šuma\n")
                wxT("z izraèunom razmerja moèi v pasovni širini B + (dBi/dBd).\n\n")
                wxT("(VC++ 2010 Express in wxWidgets-3.0.1)\n\n")
                wxT("ver2014.8.22b (c) B.Nagliè")
              );

    wxMessageBox(msg, wxT("O Spektru"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnPomoc(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( wxT("Program Spekter za izraèun razmerij moèi s poskusom prikaza:\n")
                wxT("- oblike spektra signala skupaj s šumom in\n")
                wxT("- razmerja moèi v pasovni širini sprejemnika B\n\n")
                wxT("Opis parametrov:\n\n")
                wxT("B     = pasovna širina sprejemnika (referenèna je 10) [MHz],\n")
                wxT("Ptot  = Pe = efektivna vrednost moèi skupnega signala [dBm],\n")
                wxT("Pe(B) = efektivna vrednost moèi [dBm] signala znotraj +-B/2,\n")
                wxT("Post  = efektivna vrednost moèi [dBm] signala zunaj   +-B/2,\n")
                wxT("No    = vršna vrednost gostote moèi šuma [dBm/Hz],\n")
                wxT("Ne(B) = efektivna vrednost moèi šuma [dBm] znotraj +-B/2,\n")
                wxT("S/N   = razmerje signal/šum [dB] v pasovni širini B.\n\n")
                wxT("Opis ukazov:\n\n")
                wxT("<F1>: izris trenutnega diagrama,\n")
                wxT("<F2>: vklop/izklop avtomatske izbire ref-level diagrama,\n")
                wxT("<F3>,<F4>: zmanjšanje/poveèanje No po 3dB,\n")
                wxT("<F5>,<F6>: zmanjšanje/poveèanje amplitude signala po 2dB,\n")
                wxT("<F7>,<F8>: zmanjšanje/poveèanje B s faktorjem 1/2/5,\n")
                wxT("<Dinamièen prikaz>: vklop/izklop dinamiènega prikaza šuma,\n")
                wxT("<ctrl-c>: copy - trenutni graf prenesemo v clipboard.")
            );

    wxMessageBox(msg, wxT("Pomoè za program Spekter"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnCopy(wxCommandEvent& WXUNUSED(event))
{
#if wxUSE_METAFILE && defined(wxMETAFILE_IS_ENH)
    wxMetafileDC dc;
    if (!dc.IsOk())
        return;
    m_canvas->Draw(dc);
    wxMetafile *mf = dc.Close();
    if (!mf)
        return;
    mf->SetClipboard();
    delete mf;
#endif
}

void MyFrame::OnSumPlus(wxCommandEvent& WXUNUSED(event))
{
	m_dNo *= pow(10, 3.0/10.0); //+3dB gostota moèi šuma
	if(m_dNo > 1.0e-13) m_dNo = 1.0e-13; //lim. -100 dBm/Hz
	m_canvas->Refresh();
}

void MyFrame::OnSumMinus(wxCommandEvent& WXUNUSED(event))
{
	m_dNo /= pow(10, 3.0/10.0); //-3dB gostota moèi šuma
	if(m_dNo < 4.0e-21) m_dNo = 3.98e-21; //lim. No=kT v W/Hz
	m_canvas->Refresh();
}

void MyFrame::OnNapPlus(wxCommandEvent& WXUNUSED(event))
{
	m_dAnap *= pow(10, 2.0/20.0); //+2dB amplituda
	if(m_dAnap > 0.01) m_dAnap = 0.01; //lim.
	m_canvas->Refresh();
}

void MyFrame::OnNapMinus(wxCommandEvent& WXUNUSED(event))
{
	m_dAnap /= pow(10, 2.0/20.0); //-2dB amplituda
	if(m_dAnap < 0.0000316) m_dAnap = 0.0000316; //lim. -80 dBm
	m_canvas->Refresh();
}

void MyFrame::OnBWMinus(wxCommandEvent& WXUNUSED(event))
{
	if     (m_dIFB  == 50.0)  m_dIFB = 20.0; //-1/2/5 razmerje B/10MHz
	else if(m_dIFB  == 20.0)  m_dIFB = 10.0;
	else if(m_dIFB  == 10.0)  m_dIFB = 5.0;
	else if(m_dIFB  == 5.0 )  m_dIFB = 2.0;
	else if(m_dIFB  == 2.0)   m_dIFB = 1.0;
	else if(m_dIFB  == 1.0)   m_dIFB = 0.5; 
	else if(m_dIFB  == 0.5)   m_dIFB = 0.2;
	else if(m_dIFB  == 0.2)   m_dIFB = 0.1;
	else if(m_dIFB  == 0.1)   m_dIFB = 0.05;
	else if(m_dIFB  <= 0.05)  m_dIFB = 0.02; //lim.
	m_canvas->Refresh();
}

void MyFrame::OnBWPlus(wxCommandEvent& WXUNUSED(event))
{
	if     (m_dIFB  == 0.02)  m_dIFB = 0.05; //+1/2/5 razmerje B/10MHz
	else if(m_dIFB  == 0.05)  m_dIFB = 0.1;
	else if(m_dIFB  == 0.1)   m_dIFB = 0.2;
	else if(m_dIFB  == 0.2)   m_dIFB = 0.5;
	else if(m_dIFB  == 0.5)   m_dIFB = 1.0;
	else if(m_dIFB  == 1.0)   m_dIFB = 2.0; 
	else if(m_dIFB  == 2.0)   m_dIFB = 5.0;
	else if(m_dIFB  == 5.0)   m_dIFB = 10.0;
	else if(m_dIFB  == 10.0)  m_dIFB = 20.0;
	else if(m_dIFB  >= 20.0)  m_dIFB = 50.0; //lim.
	m_canvas->Refresh();
}

//void MyFrame::OnCheckDinam(wxCommandEvent& WXUNUSED(event))
void MyFrame::OnDinam(wxCommandEvent& WXUNUSED(event))
{
	if(m_nDinam > 1) { m_nDinam = 1; } //vklop/izklop zanke dinamiènega prikaza šuma
	else             { m_nDinam = 85; }
}

void MyFrame::OnAutoRef(wxCommandEvent& WXUNUSED(event))
{
	if(m_bAutoRef == TRUE) { m_bAutoRef = FALSE; } //vklop/izklop avtomatske izbire y-skale
	else                   { m_bAutoRef = TRUE; }
}
/*
void MyFrame::OnFrek(wxCommandEvent& WXUNUSED(event))
{
	if(m_dPomik != 0) m_dPomik =  0; //menjava pomika za 1 ali 0
	else              m_dPomik = +1;
}

void MyFrame::OnBreak(wxCommandEvent& WXUNUSED(event))
{
	if(m_bBreak == TRUE) { m_bBreak = FALSE; } //vklop/izklop avtomatske izbire y-skale
	else                 { m_bBreak = TRUE; }
}
*/
void MyFrame::OnShow(wxCommandEvent& event)
{
    m_canvas->ToShow(event.GetId());
}

void MyFrame::PrepareDC(wxDC& dc)
{
    dc.SetLogicalOrigin( m_xLogicalOrigin, m_yLogicalOrigin );
    //dc.SetAxisOrientation( !m_xAxisReversed, m_yAxisReversed );
    dc.SetAxisOrientation( !m_xAxisReversed, m_yAxisReversed );
    dc.SetUserScale( m_xUserScale, m_yUserScale );
    dc.SetMapMode( m_mapMode );
}

//*
#if wxUSE_COLOURDLG
wxColour MyFrame::SelectColour()
{
    wxColour col;
    wxColourData data;
    wxColourDialog dialog(this, &data);

    if ( dialog.ShowModal() == wxID_OK )
    {
        col = dialog.GetColourData().GetColour();
    }

    return col;
}
#endif // wxUSE_COLOURDLG
//*/


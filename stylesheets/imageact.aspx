<%@ Page Language="C#" AutoEventWireup="true" CodeFile="Imageact.aspx.cs" Inherits="ImageAddNumber" %>

Imageact.aspx.cs¡¡´úÂë
public partial class ImageAddNumber : System.Web.UI.Page
{
    private static int count = 1;
    protected void Page_Load(object sender, EventArgs e)
    {
        count ;
        string pv = count.ToString();
        System.Drawing.Bitmap image = new System.Drawing.Bitmap((int)Math.Ceiling((pv.Length * 12.5)), 22);
        Graphics g = Graphics.FromImage(image);
        //Í¼Æ¬±³¾°É«
        g.Clear(Color.White);
        Font font = new System.Drawing.Font("Arial", 12, (System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic));
        System.Drawing.Drawing2D.LinearGradientBrush brush = new System.Drawing.Drawing2D.LinearGradientBrush(new Rectangle(0, 0, image.Width, image.Height), Color.White, Color.Red, (float)1.2f, true);
        g.DrawString(pv,font, brush, 0, 0);
        g.DrawRectangle(new Pen(Color.Gold), 0, 0, image.Width - 1, image.Height - 1);
        System.IO.MemoryStream ms = new System.IO.MemoryStream();
        image.Save(ms, System.Drawing.Imaging.ImageFormat.Gif);
        Response.ClearContent();
        Response.ContentType = "image/Gif";
        Response.BinaryWrite(ms.ToArray());
    }
}

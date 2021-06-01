using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace Lab3
{
    public partial class Default : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {

        }

        protected void Button2_Click(object sender, EventArgs e)
        {
            error.Visible = false;
            float far, cent;

            if (!((!string.IsNullOrWhiteSpace(TextBox1.Text)) && (!string.IsNullOrWhiteSpace(TextBox2.Text)))) //checks that only one box has a value
            {

                try
                {
                    if (string.IsNullOrWhiteSpace(TextBox1.Text) == false) //if first box has value
                    {
                        far = float.Parse(TextBox1.Text);
                        cent = (far - 32) / 1.8f;
                        TextBox2.Text = cent.ToString();
                    }
                    else //if second box has value
                    {
                        cent = float.Parse(TextBox2.Text);
                        far = cent * (9 / 5f) + 32;
                        TextBox1.Text = far.ToString();
                    }
                }
                catch
                {
                    if (int.TryParse(TextBox1.Text, out int n) || int.TryParse(TextBox2.Text, out int i) || (string.IsNullOrWhiteSpace(TextBox1.Text) && string.IsNullOrWhiteSpace(TextBox2.Text))) //if both boxes are empty or numbers
                    {
                        error.Text = "Enter one value";
                        error.Visible = true;
                    }
                    else //if input is not a number
                    {
                        error.Text = "Please enter a correct value";
                        error.Visible = true;
                    }
                }

            }
            else //if box boxes have values
            {
                error.Text = "Enter one value ";
                error.Visible = true;
            }
        }


        protected void Clear_Click(object sender, EventArgs e)
        {
            TextBox1.Text = null;
            TextBox2.Text = null;
            error.Visible = false;
        }

        protected void TextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        protected void TextBox2_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
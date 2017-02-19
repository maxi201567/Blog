function elementDisplay(obja, objb)
{
	var a=document.getElementById(obja);
	var b=document.getElementById(objb);
	if(b.style.display != 'none')
	{
		a.innerHTML='●';
		b.style.display='none';
	}
    else
	{	
        a.innerHTML='○';
        b.style.display='block';
	}
}